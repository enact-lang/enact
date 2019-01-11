#include "h/Parser.h"
#include "h/Token.h"
#include "h/Chunk.h"

Parser::Parser(std::string source) : m_source{std::move(source)}, m_scanner{m_source} {}

const ParseRule& Parser::getParseRule(TokenType type) {
    return m_parseRules[(size_t)type];
}

std::shared_ptr<Expr> Parser::parsePrecedence(Precedence precedence) {
    advance();
    PrefixFn prefixRule = getParseRule(m_previous.type).prefix;
    if (prefixRule == nullptr) {
        error("Expected expression.");
        return nullptr;
    }

    std::shared_ptr<Expr> expr = (this->*(prefixRule))();

    while (precedence <= getParseRule(m_current.type).precedence) {
        advance();
        InfixFn infixRule = getParseRule(m_previous.type).infix;
        expr = (this->*(infixRule))(expr);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::expression() {
    return parsePrecedence(Precedence::ASSIGNMENT);
}

std::shared_ptr<Expr> Parser::grouping() {
    std::shared_ptr<Expr> expr = expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    return expr;
}

std::shared_ptr<Expr> Parser::variable() {
    return std::make_shared<Expr::Variable>(m_previous);
}

std::shared_ptr<Expr> Parser::number() {
    double value = std::stod(m_previous.lexeme);
    return std::make_shared<Expr::Number>(value);
}

std::shared_ptr<Expr> Parser::literal() {
    switch (m_previous.type) {
        case TokenType::TRUE: return std::make_shared<Expr::Boolean>(true);
        case TokenType::FALSE: return std::make_shared<Expr::Boolean>(false);
        case TokenType::NIL: return std::make_shared<Expr::Nil>();
    }
}

std::shared_ptr<Expr> Parser::string() {
    return std::make_shared<Expr::String>(m_previous.lexeme.substr(1, m_previous.lexeme.size() - 2));
}

std::shared_ptr<Expr> Parser::unary() {
    Token oper = m_previous;

    std::shared_ptr<Expr> expr = parsePrecedence(Precedence::UNARY);

    return std::make_shared<Expr::Unary>(expr, oper);
}

std::shared_ptr<Expr> Parser::call(std::shared_ptr<Expr> callee) {
    Token leftParen = m_previous;
    std::vector<std::shared_ptr<Expr>> arguments;

    while (!consume(TokenType::RIGHT_PAREN)) {
        arguments.push_back(expression());
        if (!check(TokenType::RIGHT_PAREN))
            expect(TokenType::COMMA, "Expected ',' before next argument");
    }

    if (arguments.size() > UINT8_MAX) errorAt(leftParen, "Too many arguments. Max is 255.");

    return std::make_shared<Expr::Call>(callee, arguments, leftParen);
}

std::shared_ptr<Expr> Parser::binary(std::shared_ptr<Expr> left) {
    Token oper = m_previous;

    const ParseRule &rule = getParseRule(oper.type);
    std::shared_ptr<Expr> right = parsePrecedence((Precedence)((int)rule.precedence + 1));

    switch (oper.type) {
        case TokenType::AND:
        case TokenType::OR:
            return std::make_shared<Expr::Logical>(left, right, oper);

        default:
            return std::make_shared<Expr::Binary>(left, right, oper);
    }
}

std::shared_ptr<Expr> Parser::assignment(std::shared_ptr<Expr> left) {
    Token oper = m_previous;

    std::shared_ptr<Expr> right = parsePrecedence(Precedence::ASSIGNMENT);

    return std::make_shared<Expr::Assign>(left, right, oper);
}

std::shared_ptr<Expr> Parser::ternary(std::shared_ptr<Expr> condition) {
    std::shared_ptr<Expr> thenBranch = parsePrecedence(Precedence::CONDITIONAL);

    expect(TokenType::COLON, "Expected ':' after then value of conditional expression.");
    Token oper = m_previous;

    std::shared_ptr<Expr> elseBranch = parsePrecedence(Precedence::ASSIGNMENT);

    return std::make_shared<Expr::Ternary>(condition, thenBranch, elseBranch, oper);
}

std::shared_ptr<Stmt> Parser::declaration() {
    try {
        if (consume(TokenType::VAR)) return variableDeclaration(false);
        if (consume(TokenType::CONST)) return variableDeclaration(true);
        return statement();
    } catch (ParseError &error) {
        synchronise();
        return nullptr;
    }
}

std::shared_ptr<Stmt> Parser::variableDeclaration(bool isConst) {
    expect(TokenType::IDENTIFIER, "Expected variable name.");
    Token name = m_previous;

    expect(TokenType::EQUAL, "Expected '=' after variable name.");

    std::shared_ptr<Expr> initializer = expression();

    expectSeparator("Expected newline or ';' after variable declaration.");

    return std::make_shared<Stmt::Variable>(name, initializer, isConst);
}

std::shared_ptr<Stmt> Parser::statement() {
    if (consume(TokenType::SEMICOLON)) return declaration(); // Null statement;
    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::expressionStatement() {
    std::shared_ptr<Expr> expr = expression();
    expectSeparator("Expected newline or ';' after expression.");
    return std::make_shared<Stmt::Expression>(expr);
}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    advance();
    std::vector<std::shared_ptr<Stmt>> statements{};

    while (!isAtEnd()) {
        statements.push_back(declaration());
    }

    return statements;
}

void Parser::errorAt(const Token &token, const std::string &message) {
    std::cerr << "[line " << token.line << "] Error";

    if (token.type == TokenType::ENDFILE) {
        std::cerr << " at end: " << message << "\n\n";
    } else {
        if (token.type == TokenType::ERROR) {
            std::cerr << ":\n";
        } else {
            std::cerr << " at " << (token.lexeme == "\n" ? "newline" : "'" + token.lexeme + "'") << ":\n";
        }

        std::cerr << "    " << m_scanner.getSourceLine(token.lexeme == "\n" ? token.line - 1 : token.line) << "\n";
        for (int i = 1; i < token.col; ++i) {
            std::cerr << " ";
        }
        std::cerr << "    ^\n";
        std::cerr << message << "\n\n";
    }

    m_hadError = true;
    if (token.type != TokenType::ERROR) throw ParseError{};
}

void Parser::errorAtCurrent(const std::string &message) {
    errorAt(m_current, message);
}

void Parser::error(const std::string &message) {
    errorAt(m_previous, message);
}

void Parser::advance() {
    m_previous = m_current;

    while (true) {
        m_current = m_scanner.scanToken();
        if (m_current.type != TokenType::ERROR) break;

        errorAtCurrent(m_current.lexeme);
    }
}

bool Parser::check(TokenType expected) {
    return m_current.type == expected;
}

bool Parser::consume(TokenType expected) {
    if (check(expected)) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenType type, const std::string &message) {
    if (m_current.type == type) {
        advance();
    } else {
        errorAtCurrent(message);
    }
}

void Parser::expectSeparator(const std::string &message) {
    if (m_scanner.consumeSeparator()) {

    } else {
        errorAtCurrent(message);
    }
}

void Parser::synchronise() {
    advance();

    while (!isAtEnd()) {
        if (m_previous.type == TokenType::SEMICOLON) return;

        switch (m_current.type) {
            case TokenType::CLASS:
            case TokenType::CONST:
            case TokenType::FUN:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::RETURN:
            case TokenType::VAR:
            case TokenType::WHILE:
                return;
        }

        advance();
    }
}

bool Parser::isAtEnd() {
    return m_current.type == TokenType::ENDFILE;
}

bool Parser::hadError() {
    return m_hadError;
}