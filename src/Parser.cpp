#include "h/Parser.h"
#include "h/Token.h"
#include "h/Chunk.h"
#include "h/Enact.h"

Parser::Parser(std::string source) : m_source{std::move(source)}, m_scanner{m_source} {}

const ParseRule& Parser::getParseRule(TokenType type) {
    return m_parseRules[(size_t)type];
}

Expr Parser::parsePrecedence(Precedence precedence) {
    advance();
    PrefixFn prefixRule = getParseRule(m_previous.type).prefix;
    if (prefixRule == nullptr) {
        if (m_previous.type == TokenType::NEWLINE) return parsePrecedence(precedence);
        throw error("Expected expression.");
        return nullptr;
    }

    Expr expr = (this->*(prefixRule))();

    while (precedence <= getParseRule(m_current.type).precedence) {
        advance();
        InfixFn infixRule = getParseRule(m_previous.type).infix;
        expr = (this->*(infixRule))(expr);
    }

    return expr;
}

Expr Parser::expression() {
    return parsePrecedence(Precedence::ASSIGNMENT);
}

Expr Parser::grouping() {
    Expr expr = expression();
    expect(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    return expr;
}

Expr Parser::variable() {
    if (m_previous.lexeme != "_") {
        return std::make_shared<VariableExpr>(m_previous);
    } else {
        return std::make_shared<AnyExpr>();
    }
}

Expr Parser::number() {
    if (m_previous.type == TokenType::INTEGER) {
        int value = std::stoi(m_previous.lexeme);
        return std::make_shared<IntegerExpr>(value);
    }
    double value = std::stod(m_previous.lexeme);
    return std::make_shared<FloatExpr>(value);
}

Expr Parser::literal() {
    switch (m_previous.type) {
        case TokenType::TRUE: return std::make_shared<BooleanExpr>(true);
        case TokenType::FALSE: return std::make_shared<BooleanExpr>(false);
        case TokenType::NIL: return std::make_shared<NilExpr>();
    }
}

Expr Parser::string() {
    return std::make_shared<StringExpr>(m_previous.lexeme.substr(1, m_previous.lexeme.size() - 2));
}

Expr Parser::array() {
    Token square = m_previous;

    std::vector<Expr> elements;
    if (!consume(TokenType::RIGHT_SQUARE)) {
        do {
            elements.push_back(expression());
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_SQUARE, "Expected end of array.");
    }

    // Optional type name for empty array literals like []int
    std::string typeName = consumeTypeName();

    return std::make_shared<ArrayExpr>(elements, square, typeName);
}

Expr Parser::unary() {
    Token oper = m_previous;

    Expr expr = parsePrecedence(Precedence::UNARY);

    return std::make_shared<UnaryExpr>(expr, oper);
}

Expr Parser::call(Expr callee) {
    Token leftParen = m_previous;
    std::vector<Expr> arguments;

    if (!consume(TokenType::RIGHT_PAREN)) {
        do {
            arguments.push_back(expression());
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_PAREN, "Expected end of argument list.");
    }

    if (arguments.size() > 255) throw errorAt(leftParen, "Too many arguments. Max is 255.");

    return std::make_shared<CallExpr>(callee, arguments, leftParen);
}

Expr Parser::subscript(Expr object) {
    Token square = m_previous;
    Expr index = expression();
    expect(TokenType::RIGHT_SQUARE, "Expected ']' after subscript index.");

    return std::make_shared<SubscriptExpr>(object, index, square);
}

Expr Parser::binary(Expr left) {
    Token oper = m_previous;

    const ParseRule &rule = getParseRule(oper.type);
    Expr right = parsePrecedence((Precedence)((int)rule.precedence + 1));

    switch (oper.type) {
        case TokenType::AND:
        case TokenType::OR:
            return std::make_shared<LogicalExpr>(left, right, oper);

        default:
            return std::make_shared<BinaryExpr>(left, right, oper);
    }
}

Expr Parser::assignment(Expr left) {
    Token oper = m_previous;

    Expr right = parsePrecedence(Precedence::ASSIGNMENT);

    if (typeid(*left) == typeid(VariableExpr) ||
            typeid(*left) == typeid(FieldExpr) ||
            typeid(*left) == typeid(SubscriptExpr)) {
        return std::make_shared<AssignExpr>(left, right, oper);
    }

    throw errorAt(oper, "Invalid assignment target.");
}

Expr Parser::field(Expr object) {
    Token oper = m_previous;
    expect(TokenType::IDENTIFIER, "Expected field name after '.'.");
    Token name = m_previous;

    return std::make_shared<FieldExpr>(object, name, oper);
}

Expr Parser::ternary(Expr condition) {
    Expr thenBranch = parsePrecedence(Precedence::CONDITIONAL);

    expect(TokenType::COLON, "Expected ':' after then value of conditional expression.");
    Token oper = m_previous;

    Expr elseBranch = parsePrecedence(Precedence::ASSIGNMENT);

    return std::make_shared<TernaryExpr>(condition, thenBranch, elseBranch, oper);
}

Stmt Parser::declaration() {
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

Stmt Parser::functionDeclaration(bool mustParseBody) {
    expect(TokenType::IDENTIFIER, "Expected function name.");
    Token name = m_previous;

    expect(TokenType::LEFT_PAREN, "Expected '(' after function name.");

    std::vector<NamedTypename> params;
    if (!consume(TokenType::RIGHT_PAREN)) {
        do {
            expect(TokenType::IDENTIFIER, "Expected parameter name.");
            params.push_back(NamedTypename{m_previous, consumeTypeName()});
        } while (consume(TokenType::COMMA));

        expect(TokenType::RIGHT_PAREN, "Expected end of parameter list.");
    }

    // Get the return type
    std::string typeName = consumeTypeName();

    std::vector<Stmt> body;

    if (!mustParseBody && consumeSeparator()) {
        return std::make_shared<FunctionStmt>(name, typeName, params, body);
    }

    expect(TokenType::COLON, "Expected ':' before function body.");
    ignoreNewline();

    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of function declaration.");

    expectSeparator("Expected newline or ';' after function declaration.");

    return std::make_shared<FunctionStmt>(name, typeName, params, body);
}

Stmt Parser::structDeclaration() {
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

    std::vector<NamedTypename> fields;
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    std::vector<std::shared_ptr<FunctionStmt>> assocFunctions;

    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();
        if (consume(TokenType::IDENTIFIER)) {
            // Field declaration
            Token fieldName = m_previous;
            std::string fieldType = consumeTypeName();

            fields.push_back(NamedTypename{fieldName, fieldType});

            expectSeparator("Expected newline or ';' after field declaration.");
        } else if (consume(TokenType::FUN)) {
            // Method declaration
            std::shared_ptr<FunctionStmt> method = std::static_pointer_cast<FunctionStmt>(functionDeclaration());
            methods.push_back(method);
        } else if (consume(TokenType::ASSOC)) {
            // Associated function declaration
            auto function = std::static_pointer_cast<FunctionStmt>(functionDeclaration());
            assocFunctions.push_back(function);
        } else {
            throw errorAtCurrent("Expected field or method declaration.");
        }
    }

    expect(TokenType::END, "Expected 'end' at end of struct declaration.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<StructStmt>(name, traits, fields, methods, assocFunctions);
}

Stmt Parser::traitDeclaration() {
    expect(TokenType::IDENTIFIER, "Expected trait name.");
    Token name = m_previous;

    expect(TokenType::COLON, "Expected ':' after trait name.");
    ignoreNewline();

    std::vector<std::shared_ptr<FunctionStmt>> methods;
    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();
        if (consume(TokenType::FUN)) {
            auto method = std::static_pointer_cast<FunctionStmt>(functionDeclaration(false));
            methods.push_back(method);
        } else {
            throw errorAtCurrent("Expected method declaration.");
        }
    }

    expect(TokenType::END, "Expected 'end' at end of trait declaration.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<TraitStmt>(name, methods);
}

Stmt Parser::variableDeclaration(bool isConst) {
    expect(TokenType::IDENTIFIER, "Expected variable name.");
    Token name = m_previous;

    std::string typeName { consumeTypeName() };

    expect(TokenType::EQUAL, "Expected '=' after variable name/type.");

    Expr initializer = expression();

    expectSeparator("Expected newline or ';' after variable declaration.");

    return std::make_shared<VariableStmt>(name, typeName, initializer, isConst);
}

Stmt Parser::statement() {
    if (consume(TokenType::BLOCK)) return blockStatement();
    if (consume(TokenType::IF)) return ifStatement();
    if (consume(TokenType::WHILE)) return whileStatement();
    if (consume(TokenType::FOR)) return forStatement();
    if (consume(TokenType::EACH)) return eachStatement();
    if (consume(TokenType::GIVEN)) return givenStatement();
    if (consume(TokenType::RETURN)) return returnStatement();
    if (consume(TokenType::BREAK)) return breakStatement();
    if (consume(TokenType::CONTINUE)) return continueStatement();
    return expressionStatement();
}

Stmt Parser::blockStatement() {
    expect(TokenType::COLON, "Expected ':' before block body.");
    ignoreNewline();

    std::vector<Stmt> statements;
    while (!check(TokenType::END) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of block.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<BlockStmt>(statements);
}

Stmt Parser::ifStatement() {
    Token keyword = m_previous;

    Expr condition = expression();
    expect(TokenType::COLON, "Expected ':' after if condition.");
    ignoreNewline();

    std::vector<Stmt> thenBlock;
    while (!check(TokenType::END) && !check(TokenType::ELSE) && !isAtEnd()) {
        thenBlock.push_back(declaration());
    }

    std::vector<Stmt> elseBlock;

    if (consume(TokenType::ELSE)) {
        expect(TokenType::COLON, "Expected ':' after start of else block.");
        ignoreNewline();
        while (!check(TokenType::END) && !isAtEnd()) {
            elseBlock.push_back(declaration());
        }
    }

    expect(TokenType::END, "Expected 'end' at end of if statement.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<IfStmt>(condition, thenBlock, elseBlock, keyword);
}

Stmt Parser::whileStatement() {
    Token keyword = m_previous;

    Expr condition = expression();
    
    expect(TokenType::COLON, "Expected ':' after while condition.");
    ignoreNewline();

    std::vector<Stmt> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of while loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<WhileStmt>(condition, body, keyword);
}

Stmt Parser::forStatement() {
    Token keyword = m_previous;

    Stmt initializer;
    if (consume(TokenType::SEMICOLON)) {
        initializer = std::make_shared<ExpressionStmt>(std::make_shared<NilExpr>());
    } else if (consume(TokenType::VAR)) {
        initializer = variableDeclaration(false);
    } else if (consume(TokenType::CONST)) {
        initializer = variableDeclaration(true);
    } else {
        initializer = expressionStatement();
    }

    Expr condition;
    if (!consume(TokenType::SEMICOLON)) {
        condition = expression();
        expect(TokenType::SEMICOLON, "Expected ';' after for loop condition.");
    } else {
        condition = std::make_shared<BooleanExpr>(true);
    }

    Expr increment;
    if (!check(TokenType::COLON)) {
        increment = expression();
    } else {
        increment = std::make_shared<NilExpr>();
    }

    expect(TokenType::COLON, "Expected ':' before body of for loop.");

    ignoreNewline();

    std::vector<Stmt> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of for loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<ForStmt>(initializer, condition, increment, body, keyword);
}

Stmt Parser::eachStatement() {
    expect(TokenType::IDENTIFIER, "Expected item name after 'each'.");
    Token name = m_previous;

    expect(TokenType::IN, "Expected 'in' after each loop item name.");

    Expr object = expression();

    expect(TokenType::COLON, "Expected ':' before each loop body.");
    ignoreNewline();

    std::vector<Stmt> body;
    while (!check(TokenType::END) && !isAtEnd()) {
        body.push_back(declaration());
    }

    expect(TokenType::END, "Expected 'end' at end of each loop.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<EachStmt>(name, object, body);
}

Stmt Parser::givenStatement() {
    Expr value = expression();
    expect(TokenType::COLON, "Expected ':' before given statement body.");

    std::vector<GivenCase> cases;
    while (!check(TokenType::END) && !isAtEnd()) {
        ignoreNewline();

        if (consume(TokenType::WHEN)) {
            Token keyword = m_previous;

            Expr caseValue = expression();
            expect(TokenType::COLON, "Expected ':' before case body");

            std::vector<Stmt> caseBody;
            while (!check(TokenType::WHEN) && !check(TokenType::ELSE) && !check(TokenType::END) && !isAtEnd()) {
                caseBody.push_back(declaration());
            }

            cases.push_back(GivenCase{caseValue, caseBody, keyword});
        } else if (consume(TokenType::ELSE)) {
            Token keyword = m_previous;

            expect(TokenType::COLON, "Expected ':' before 'else' case body.");

            std::vector<Stmt> caseBody;
            while (!check(TokenType::WHEN) && !check(TokenType::END) && !isAtEnd()) {
                caseBody.push_back(declaration());
            }

            Expr caseValue = std::make_shared<AnyExpr>();

            cases.push_back(GivenCase{caseValue, caseBody, keyword});
        }
    }

    expect(TokenType::END, "Expected 'end' at end of given statement.");
    expectSeparator("Expected newline or ';' after 'end'.");

    return std::make_shared<GivenStmt>(value, cases);
}

Stmt Parser::returnStatement() {
    Token keyword = m_previous;
    Expr value = expression();

    expectSeparator("Expected newline or ';' after return statement.");

    return std::make_shared<ReturnStmt>(keyword, value);
}

Stmt Parser::breakStatement() {
    expectSeparator("Expected newline or ';' after break statement.");
    return std::make_shared<BreakStmt>(m_previous);
}

Stmt Parser::continueStatement() {
    expectSeparator("Expected newline or ';' after continue statement.");
    return std::make_shared<ContinueStmt>(m_previous);
}

Stmt Parser::expressionStatement() {
    Expr expr = expression();
    expectSeparator("Expected newline or ';' after expression.");
    return std::make_shared<ExpressionStmt>(expr);
}

std::vector<Stmt> Parser::parse() {
    advance();
    std::vector<Stmt> statements{};

    while (!isAtEnd()) {
        Stmt stmt = declaration();
        if (stmt) statements.push_back(stmt);
    }

    return statements;
}

Parser::ParseError Parser::errorAt(const Token &token, const std::string &message) {
    Enact::reportErrorAt(token, message);
    m_hadError = true;
    return ParseError{};
}

Parser::ParseError Parser::errorAtCurrent(const std::string &message) {
    return errorAt(m_current, message);
}

Parser::ParseError Parser::error(const std::string &message) {
    return errorAt(m_previous, message);
}

void Parser::advance() {
    m_previous = m_current;

    while (true) {
        m_current = m_scanner.scanToken();
        if (m_current.type != TokenType::ERROR) break;

        Enact::reportErrorAt(m_current, m_current.lexeme);
    }
}

void Parser::undoAdvance() {
    m_current = m_previous;
    m_previous = m_scanner.backtrack();
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
        throw errorAtCurrent(message);
    }
}

void Parser::expectSeparator(const std::string &message) {
    if (consumeSeparator()) return;
    throw errorAtCurrent(message);
}

std::string Parser::consumeTypeName() {
    std::string typeName;

    // May be enclosed in square brackets to signify list type
    if (consume(TokenType::LEFT_SQUARE)) {
        typeName += "[";

        std::string elementType = consumeTypeName();
        if (elementType.empty()) {
            undoAdvance();
            return "";
        }

        typeName += elementType;

        expect(TokenType::RIGHT_SQUARE, "Expected ']' after list type name.");

        typeName += "]";
        return typeName;
    }

    if (consume(TokenType::IDENTIFIER)) {
        typeName += m_previous.lexeme;
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