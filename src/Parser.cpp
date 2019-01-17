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
    if (m_previous.lexeme != "_") {
        return std::make_shared<Expr::Variable>(m_previous);
    } else {
        return std::make_shared<Expr::Any>();
    }
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
        if (consume(TokenType::FUN)) return functionDeclaration();
        if (consume(TokenType::STRUCT)) return structDeclaration();
        if (consume(TokenType::TRAIT)) return traitDeclaration();
        if (consume(TokenType::VAR)) return variableDeclaration(false);
        if (consume(TokenType::CONST)) return variableDeclaration(true);
        return statement();
    } catch (ParseError &error) {
        synchronise();
        return nullptr;
    }
}

std::shared_ptr<Stmt> Parser::functionDeclaration(bool mustParseBody) {
    expect(TokenType::IDENTIFIER, "Expected function name.");
    Token name = m_previous;

    expect(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<Parameter> params;
    if (!consume(TokenType::RIGHT_PAREN)) {
        do {
            expect(TokenType::IDENTIFIER, "Expected parameter name.");
            params.push_back(Parameter{m_previous, consumeTypeName()});
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_PAREN, "Expected end of parameter list.");
    }

    // Get the return type
    std::string typeName = consumeTypeName();

    std::vector<std::shared_ptr<Stmt>> body;

    if (!mustParseBody && consumeSeparator()) {
        return std::make_shared<Stmt::Function>(name, typeName, params, body);
    }

    expect(TokenType::COLON, "Expected ':' before function body.");
    ignoreNewline();

    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of function declaration.");

    expectSeparator("Expected newline or ';' after function declaration.");

    return std::make_shared<Stmt::Function>(name, typeName, params, body);
}

std::shared_ptr<Stmt> Parser::structDeclaration() {
    expect(TokenType::IDENTIFIER, "Expected struct name.");
    Token name = m_previous;

    std::vector<Token> traits;
    if (consume(TokenType::IS)) {
        expect(TokenType::IDENTIFIER, "Expected trait name.");
        traits.push_back(m_previous);

        while (consume(TokenType::COMMA)) {
            expect(TokenType::IDENTIFIER, "Expected trait name.");
            traits.push_back(m_previous);
        }
    }

    expect(TokenType::COLON, "Expected ':' before struct body.");
    ignoreNewline();

    std::vector<Field> fields;
    std::vector<std::shared_ptr<Stmt::Function>> methods;
    std::vector<std::shared_ptr<Stmt::Function>> assocFunctions;

    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();
        if (consume(TokenType::IDENTIFIER)) {
            // Field declaration
            Token fieldName = m_previous;
            std::string fieldType = consumeTypeName();

            fields.push_back(Field{m_previous, fieldType});

            expectSeparator("Expected newline or ';' after field declaration.");
        } else if (consume(TokenType::FUN)) {
            // Method declaration
            std::shared_ptr<Stmt::Function> method = std::static_pointer_cast<Stmt::Function>(functionDeclaration());
            methods.push_back(method);
        } else if (consume(TokenType::ASSOC)) {
            // Associated function declaration
            auto function = std::static_pointer_cast<Stmt::Function>(functionDeclaration());
            assocFunctions.push_back(function);
        } else {
            errorAtCurrent("Expected field or method declaration.");
        }
    }

    expect(TokenType::END, "Expected 'end' at end of struct declaration.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Struct>(name, traits, fields, methods, assocFunctions);
}

std::shared_ptr<Stmt> Parser::traitDeclaration() {
    expect(TokenType::IDENTIFIER, "Expected trait name.");
    Token name = m_previous;

    expect(TokenType::COLON, "Expected ':' after trait name.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt::Function>> methods;
    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();
        if (consume(TokenType::FUN)) {
            auto method = std::static_pointer_cast<Stmt::Function>(functionDeclaration(false));
            methods.push_back(method);
        } else {
            errorAtCurrent("Expected method declaration.");
        }
    }

    expect(TokenType::END, "Expected 'end' at end of trait declaration.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Trait>(name, methods);
}

std::shared_ptr<Stmt> Parser::variableDeclaration(bool isConst) {
    expect(TokenType::IDENTIFIER, "Expected variable name.");
    Token name = m_previous;

    std::string typeName { consumeTypeName() };

    expect(TokenType::EQUAL, "Expected '=' after variable name/type.");

    std::shared_ptr<Expr> initializer = expression();

    expectSeparator("Expected newline or ';' after variable declaration.");

    return std::make_shared<Stmt::Variable>(name, typeName, initializer, isConst);
}

std::shared_ptr<Stmt> Parser::statement() {
    if (consume(TokenType::BLOCK)) return blockStatement();
    if (consume(TokenType::EACH)) return eachStatement();
    if (consume(TokenType::FOR)) return forStatement();
    if (consume(TokenType::GIVEN)) return givenStatement();
    if (consume(TokenType::IF)) return ifStatement();
    if (consume(TokenType::WHILE)) return whileStatement();
    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::blockStatement() {
    expect(TokenType::COLON, "Expected ':' before block body.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::END) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of block.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Block>(statements);
}

std::shared_ptr<Stmt> Parser::eachStatement() {
    expect(TokenType::IDENTIFIER, "Expected item name after 'each'.");
    Token name = m_previous;

    expect(TokenType::IN, "Expected 'in' after each loop item name.");

    std::shared_ptr<Expr> object = expression();

    expect(TokenType::COLON, "Expected ':' before each loop body.");
    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of each loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Each>(name, object, body);
}

std::shared_ptr<Stmt> Parser::forStatement() {
    std::shared_ptr<Stmt> initializer;
    if (consume(TokenType::SEMICOLON)) {
        initializer = std::make_shared<Stmt::Expression>(std::make_shared<Expr::Nil>());
    } else if (consume(TokenType::VAR)) {
        initializer = variableDeclaration(false);
    } else if (consume(TokenType::CONST)) {
        initializer = variableDeclaration(true);
    } else {
        initializer = expressionStatement();
    }

    std::shared_ptr<Expr> condition;
    if (!consume(TokenType::SEMICOLON)) {
        condition = expression();
        expect(TokenType::SEMICOLON, "Expected ';' after for loop condition.");
    } else {
        condition = std::make_shared<Expr::Boolean>(true);
    }

    std::shared_ptr<Expr> increment;
    if (!check(TokenType::COLON)) {
        increment = expression();
    } else {
        increment = std::make_shared<Expr::Nil>();
    }

    expect(TokenType::COLON, "Expected ':' before body of for loop.");

    ignoreNewline();

    std::vector<std::shared_ptr<Stmt>> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of for loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::For>(initializer, condition, increment, body);
}

std::shared_ptr<Stmt> Parser::givenStatement() {
    std::shared_ptr<Expr> value = expression();
    expect(TokenType::COLON, "Expected ':' before given statement body.");

    std::vector<GivenCase> cases;
    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();

        if (consume(TokenType::WHEN)) {
            std::shared_ptr<Expr> caseValue = expression();
            expect(TokenType::COLON, "Expected ':' before case body");

            std::vector<std::shared_ptr<Stmt>> caseBody;
            while (!check(TokenType::WHEN) && !check(TokenType::ELSE) && !check(TokenType::END) && !isAtEnd()) {
                caseBody.push_back(declaration());
            }

            cases.push_back(GivenCase{caseValue, caseBody});
        } else if (consume(TokenType::ELSE)) {
            expect(TokenType::COLON, "Expected ':' before 'else' case body.");

            std::vector<std::shared_ptr<Stmt>> caseBody;
            while (!check(TokenType::WHEN) && !check(TokenType::END) && !isAtEnd()) {
                caseBody.push_back(declaration());
            }

            std::shared_ptr<Expr> caseValue = std::make_shared<Expr::Any>();

            cases.push_back(GivenCase{caseValue, caseBody});
        }
    }

    expect(TokenType::END, "Expected 'end' at end of given statement.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<Stmt::Given>(value, cases);
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

    expect(TokenType::END, "Expected 'end' at end of while loop.");
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

bool Parser::consumeSeparator() {
    return consume(TokenType::NEWLINE) || consume(TokenType::SEMICOLON);
}

void Parser::expect(TokenType type, const std::string &message) {
    if (m_current.type == type) {
        advance();
    } else {
        errorAtCurrent(message);
    }
}

void Parser::expectSeparator(const std::string &message) {
    if (consumeSeparator()) return;
    errorAtCurrent(message);
}

std::string Parser::consumeTypeName() {
    std::string typeName;

    // May be enclosed in square brackets to signify list type
    bool isList = false;
    if (consume(TokenType::LEFT_SQUARE)) {
        isList = true;
        typeName += "[";
    }

    // May start with 'ref'
    if (consume(TokenType::REF)) {
        expect(TokenType::IDENTIFIER, "Expected rest of type name after 'ref'.");
        typeName += "ref " + m_previous.lexeme;
    } else if (consume(TokenType::IDENTIFIER)) {
        typeName += m_previous.lexeme;
    }

    // Expect the closing ']' if we were consuming a list type.
    if (isList) {
        expect(TokenType::RIGHT_SQUARE, "Expected ']' after list type name.");
        typeName += "]";
    }

    return typeName;
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