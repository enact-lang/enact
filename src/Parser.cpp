#include "h/Parser.h"
#include "h/Token.h"
#include "h/Chunk.h"

Parser::Parser(std::string source) : m_source{std::move(source)}, m_scanner{m_source} {}

const ParseRule& Parser::getParseRule(TokenType type) {
    return m_parseRules[(size_t)type];
}

Sp<Expr> Parser::parsePrecedence(Precedence precedence) {
    advance();
    PrefixFn prefixRule = getParseRule(m_previous.type).prefix;
    if (prefixRule == nullptr) {
        error("Expected expression.");
        return nullptr;
    }

    Sp<Expr> expr = (this->*(prefixRule))();

    while (precedence <= getParseRule(m_current.type).precedence) {
        advance();
        InfixFn infixRule = getParseRule(m_previous.type).infix;
        expr = (this->*(infixRule))(expr);
    }

    return expr;
}

Sp<Expr> Parser::expression() {
    return parsePrecedence(Precedence::ASSIGNMENT);
}

Sp<Expr> Parser::grouping() {
    Sp<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    return expr;
}

Sp<Expr> Parser::variable() {
    return std::make_shared<Expr::Variable>(m_previous);
}

Sp<Expr> Parser::number() {
    double value = std::stod(m_previous.lexeme);
    return std::make_shared<Expr::Number>(value);
}

Sp<Expr> Parser::literal() {
    switch (m_previous.type) {
        case TokenType::TRUE: return std::make_shared<Expr::Boolean>(true);
        case TokenType::FALSE: return std::make_shared<Expr::Boolean>(false);
        case TokenType::NIL: return std::make_shared<Expr::Nil>();
    }
}

Sp<Expr> Parser::string() {
    return std::make_shared<Expr::String>(m_previous.lexeme.substr(1, m_previous.lexeme.size() - 2));
}

Sp<Expr> Parser::unary() {
    Token oper = m_previous;

    Sp<Expr> expr = parsePrecedence(Precedence::UNARY);

    return std::make_shared<Expr::Unary>(expr, oper);
}

Sp<Expr> Parser::call(Sp<Expr> callee) {
    Token leftParen = m_previous;
    std::vector<Sp<Expr>> arguments;

    while (!match(TokenType::RIGHT_PAREN)) {
        arguments.push_back(expression());
        if (!check(TokenType::RIGHT_PAREN))
            consume(TokenType::COMMA, "Expected ',' before next argument");
    }

    if (arguments.size() > UINT8_MAX) errorAt(leftParen, "Too many arguments. Max is 255.");

    return std::make_shared<Expr::Call>(callee, arguments, leftParen);
}

Sp<Expr> Parser::binary(Sp<Expr> left) {
    Token oper = m_previous;

    const ParseRule &rule = getParseRule(oper.type);
    Sp<Expr> right = parsePrecedence(rule.precedence);

    if (oper.type == TokenType::EQUAL) {
        return std::make_shared<Expr::Assign>(left, right, oper);
    }

    return std::make_shared<Expr::Binary>(left, right, oper);
}

Sp<Expr> Parser::ternary(Sp<Expr> condition) {
    Sp<Expr> thenBranch = parsePrecedence(Precedence::CONDITIONAL);

    consume(TokenType::COLON, "Expected ':' after then value of conditional expression.");
    Token oper = m_previous;

    Sp<Expr> elseBranch = parsePrecedence(Precedence::ASSIGNMENT);

    return std::make_shared<Expr::Ternary>(condition, thenBranch, elseBranch, oper);
}

Sp<Stmt> Parser::declaration() {
    if (match(TokenType::VAR)) return variableDeclaration(false);
    if (match(TokenType::CONST)) return variableDeclaration(true);
    return statement();
}

Sp<Stmt> Parser::variableDeclaration(bool isConst) {
    consume(TokenType::IDENTIFIER, "Expected variable name.");
    Token name = m_previous;

    consume(TokenType::EQUAL, "Expected '=' after variable name.");

    Sp<Expr> initializer = expression();

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

    return std::make_shared<Stmt::Variable>(name, initializer, isConst);
}

Sp<Stmt> Parser::statement() {
    if (match(TokenType::PRINT)) return printStatement();
    if (match(TokenType::SEMICOLON)) return statement(); // Null statement;
    return expressionStatement();
}

Sp<Stmt> Parser::printStatement() {
    Sp<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after print statement.");
    return std::make_shared<Stmt::Print>(expr);
}

Sp<Stmt> Parser::expressionStatement() {
    Sp<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_shared<Stmt::Expression>(expr);
}

std::vector<Sp<Stmt>> Parser::parse() {
    advance();
    std::vector<Sp<Stmt>> statements{};

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
            std::cerr << " at '" << token.lexeme << "':\n";
        }

        std::cerr << "    " << m_scanner.getSourceLine(token.line) << "\n";
        for (int i = 1; i < token.col; ++i) {
            std::cerr << " ";
        }
        std::cerr << "    ^\n";
        std::cerr << message << "\n\n";
    }

    m_hadError = true;
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

bool Parser::match(TokenType expected) {
    if (check(expected)) {
        advance();
        return true;
    }
    return false;
}

void Parser::consume(TokenType type, const std::string &message) {
    if (m_current.type == type) {
        advance();
    } else {
        errorAtCurrent(message);
    }
}

bool Parser::isAtEnd() {
    return m_current.type == TokenType::ENDFILE;
}

bool Parser::hadError() {
    return m_hadError;
}