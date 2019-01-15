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
        if (m_previous.type == TokenType::NEWLINE) return parsePrecedence(precedence);
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

std::shared_ptr<Expr> Parser::array() {
    std::vector<std::shared_ptr<Expr>> elements;
    if (!consume(TokenType::RIGHT_SQUARE)) {
        do {
            elements.push_back(expression());
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_SQUARE, "Expected end of array.");
    }

    return std::make_shared<Expr::Array>(elements);
}

std::shared_ptr<Expr> Parser::unary() {
    Token oper = m_previous;

    std::shared_ptr<Expr> expr = parsePrecedence(Precedence::UNARY);

    if (oper.type == TokenType::REF) {
        if (typeid(*expr) != typeid(Expr::Variable) &&
                typeid(*expr) != typeid(Expr::Field) &&
                typeid(*expr) != typeid(Expr::Subscript)) {
            errorAt(oper, "Can only reference lvalues.");
        }

        return std::make_shared<Expr::Reference>(expr, oper);
    }

    return std::make_shared<Expr::Unary>(expr, oper);
}

std::shared_ptr<Expr> Parser::call(std::shared_ptr<Expr> callee) {
    Token leftParen = m_previous;
    std::vector<std::shared_ptr<Expr>> arguments;

    if (!consume(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_PAREN, "Expected end of argument list.");
    }

    if (arguments.size() > 255) errorAt(leftParen, "Too many arguments. Max is 255.");

    return std::make_shared<Expr::Call>(callee, arguments, leftParen);
}

std::shared_ptr<Expr> Parser::subscript(std::shared_ptr<Expr> object) {
    Token square = m_previous;
    std::shared_ptr<Expr> index = expression();
    expect(TokenType::RIGHT_SQUARE, "Expected ']' after subscript index.");

    return std::make_shared<Expr::Subscript>(object, index, square);
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

    if (typeid(*left) == typeid(Expr::Variable) ||
            typeid(*left) == typeid(Expr::Field) ||
            typeid(*left) == typeid(Expr::Subscript)) {
        return std::make_shared<Expr::Assign>(left, right, oper);
    }

    errorAt(oper, "Invalid assignment target.");
}

std::shared_ptr<Expr> Parser::field(std::shared_ptr<Expr> object) {
    Token oper = m_previous;
    expect(TokenType::IDENTIFIER, "Expected field name after '.'.");
    Token name = m_previous;

    return std::make_shared<Expr::Field>(object, name, oper);
}

std::shared_ptr<Expr> Parser::ternary(std::shared_ptr<Expr> condition) {
    std::shared_ptr<Expr> thenBranch = parsePrecedence(Precedence::CONDITIONAL);

    expect(TokenType::COLON, "Expected ':' after then value of conditional expression.");
    Token oper = m_previous;

    std::shared_ptr<Expr> elseBranch = parsePrecedence(Precedence::ASSIGNMENT);

    return std::make_shared<Expr::Ternary>(condition, thenBranch, elseBranch, oper);
}

std::shared_ptr<Stmt> Parser::declaration() {
    ignoreNewline();
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

    std::string typeName;
    if (consume(TokenType::REF)) {
        expect(TokenType::IDENTIFIER, "Expected rest of type name after 'ref'.");
        typeName = "ref " + m_previous.lexeme;
    } else if (consume(TokenType::IDENTIFIER)) {
        typeName = m_previous.lexeme;
    }

    expect(TokenType::EQUAL, "Expected '=' after variable name/type.");

    std::shared_ptr<Expr> initializer = expression();

    expectSeparator("Expected newline or ';' after variable declaration.");

    return std::make_shared<Stmt::Variable>(name, typeName, initializer, isConst);
}

std::shared_ptr<Stmt> Parser::statement() {
    if (consume(TokenType::BLOCK)) return blockStatement();
    if (consume(TokenType::FOR)) return forStatement();
    if (consume(TokenType::IF)) return ifStatement();
    if (consume(TokenType::WHILE)) return whileStatement();
    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::blockStatement() {
    expect(TokenType::COLON, "Expected ':' after start of block.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::END) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of block.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Block>(statements);
}

std::shared_ptr<Stmt> Parser::forStatement() {
    std::shared_ptr<Stmt> initializer;
    if (consume(TokenType::SEMICOLON)) {
        initializer = nullptr;
    } else if (consume(TokenType::VAR)) {
        initializer = variableDeclaration(false);
    } else if (consume(TokenType::CONST)) {
        initializer = variableDeclaration(true);
    } else {
        initializer = expressionStatement();
    }

    std::shared_ptr<Expr> condition{nullptr};
    if (!consume(TokenType::SEMICOLON)) {
        condition = expression();
        expect(TokenType::SEMICOLON, "Expected ';' after for loop condition.");
    }

    std::shared_ptr<Expr> increment{nullptr};
    if (!check(TokenType::COLON)) {
        increment = expression();
    }

    expect(TokenType::COLON, "Expected ':' before body of for loop.");

    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of for loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    // A for loop is just syntactic sugar for a while loop inside a
    // block, so we create the "real" representation now.
    if (increment != nullptr) {
        body.push_back(std::make_shared<Stmt::Expression>(increment));
    }

    std::vector<std::shared_ptr<Stmt>> outerBody;

    if (initializer != nullptr) {
        outerBody.push_back(initializer);
    }

    if (condition != nullptr) {
        outerBody.push_back(std::make_shared<Stmt::While>(condition, body));
    } else {
        outerBody.push_back(std::make_shared<Stmt::While>(std::make_shared<Expr::Boolean>(true), body));
    }

    return std::make_shared<Stmt::Block>(outerBody);
}

std::shared_ptr<Stmt> Parser::ifStatement() {
    std::shared_ptr<Expr> condition = expression();
    expect(TokenType::COLON, "Expected ':' after if condition.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> thenBlock;
    while (!check(TokenType::END) && !check(TokenType::ELSE) && !isAtEnd()) {
        thenBlock.push_back(declaration());
    }

    std::vector<std::shared_ptr<Stmt>> elseBlock;

    if (consume(TokenType::ELSE)) {
        expect(TokenType::COLON, "Expected ':' after start of else block.");
        ignoreNewline();
        while (!check(TokenType::END) && !isAtEnd()) {
            elseBlock.push_back(declaration());
        }
    }

    expect(TokenType::END, "Expected 'end' at end of if statement.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::If>(condition, thenBlock, elseBlock);
}

std::shared_ptr<Stmt> Parser::whileStatement() {
    std::shared_ptr<Expr> condition = expression();
    expect(TokenType::COLON, "Expected ':' after while condition.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of while statement.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::While>(condition, body);
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

void Parser::ignoreNewline() {
    while (consume(TokenType::NEWLINE));
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
    if (consume(TokenType::NEWLINE) || consume(TokenType::SEMICOLON)) return;
    errorAtCurrent(message);
}

void Parser::synchronise() {
    advance();

    while (!isAtEnd()) {
        switch (m_current.type) {
            case TokenType::BLOCK:
            case TokenType::CLASS:
            case TokenType::CONST:
            case TokenType::EACH:
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