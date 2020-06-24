#include "../context/CompileContext.h"

#include "Token.h"

namespace enact {
    Parser::Parser(CompileContext &context) : m_context{context} {
    }

    const ParseRule &Parser::getParseRule(TokenType type) {
        return m_parseRules[(size_t) type];
    }

    std::unique_ptr<Expr> Parser::parsePrecedence(Precedence precedence) {
        advance();
        PrefixFn prefixRule = getParseRule(m_previous.type).prefix;
        if (prefixRule == nullptr) {
            if (m_previous.type == TokenType::NEWLINE) return parsePrecedence(precedence);
            throw error("Expected expression.");
            return nullptr;
        }

        std::unique_ptr<Expr> expr = (this->*(prefixRule))();

        while (precedence <= getParseRule(m_current.type).precedence) {
            advance();
            InfixFn infixRule = getParseRule(m_previous.type).infix;
            expr = (this->*(infixRule))(std::move(expr));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::expression() {
        return parsePrecedence(Precedence::ASSIGNMENT);
    }

    std::unique_ptr<Expr> Parser::grouping() {
        std::unique_ptr<Expr> expr = expression();
        expect(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return expr;
    }

    std::unique_ptr<Expr> Parser::variable() {
        if (m_previous.lexeme != "_") {
            return std::make_unique<VariableExpr>(m_previous);
        } else {
            return std::make_unique<AnyExpr>();
        }
    }

    std::unique_ptr<Expr> Parser::number() {
        if (m_previous.type == TokenType::INTEGER) {
            int value = std::stoi(m_previous.lexeme);
            return std::make_unique<IntegerExpr>(value);
        }
        double value = std::stod(m_previous.lexeme);
        return std::make_unique<FloatExpr>(value);
    }

    std::unique_ptr<Expr> Parser::literal() {
        switch (m_previous.type) {
            case TokenType::TRUE:
                return std::make_unique<BooleanExpr>(true);
            case TokenType::FALSE:
                return std::make_unique<BooleanExpr>(false);
            case TokenType::NIL:
                return std::make_unique<NilExpr>();
        }
    }

    std::unique_ptr<Expr> Parser::string() {
        return std::make_unique<StringExpr>(m_previous.lexeme.substr(1, m_previous.lexeme.size() - 2));
    }

    std::unique_ptr<Expr> Parser::array() {
        Token square = m_previous;

        std::unique_ptr<const Typename> typeName;

        std::vector<std::unique_ptr<Expr>> elements;
        if (!consume(TokenType::RIGHT_SQUARE)) {
            do {
                elements.push_back(expression());
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_SQUARE, "Expected end of array.");

            // Typename can be empty
            typeName = expectTypename(true);
        } else {
            // Typename cannot be empty
            typeName = expectTypename();
        }

        return std::make_unique<ArrayExpr>(std::move(elements), square, std::move(typeName));
    }

    std::unique_ptr<Expr> Parser::unary() {
        Token oper = m_previous;

        std::unique_ptr<Expr> expr = parsePrecedence(Precedence::UNARY);

        return std::make_unique<UnaryExpr>(std::move(expr), oper);
    }

    std::unique_ptr<Expr> Parser::call(std::unique_ptr<Expr> callee) {
        Token leftParen = m_previous;
        std::vector<std::unique_ptr<Expr>> arguments;

        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                arguments.push_back(expression());
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of argument list.");
        }

        if (arguments.size() > 255) throw errorAt(leftParen, "Too many arguments. Max is 255.");

        return std::make_unique<CallExpr>(std::move(callee), std::move(arguments), leftParen);
    }

    std::unique_ptr<Expr> Parser::subscript(std::unique_ptr<Expr> object) {
        Token square = m_previous;
        std::unique_ptr<Expr> index = expression();
        expect(TokenType::RIGHT_SQUARE, "Expected ']' after subscript index.");

        return std::make_unique<SubscriptExpr>(std::move(object), std::move(index), square);
    }

    std::unique_ptr<Expr> Parser::binary(std::unique_ptr<Expr> left) {
        Token oper = m_previous;

        const ParseRule &rule = getParseRule(oper.type);
        std::unique_ptr<Expr> right = parsePrecedence((Precedence) ((int) rule.precedence + 1));

        switch (oper.type) {
            case TokenType::AND:
            case TokenType::OR:
                return std::make_unique<LogicalExpr>(std::move(left), std::move(right), oper);

            default:
                return std::make_unique<BinaryExpr>(std::move(left), std::move(right), oper);
        }
    }

    std::unique_ptr<Expr> Parser::assignment(std::unique_ptr<Expr> target) {
        Token oper = m_previous;

        std::unique_ptr<Expr> value = parsePrecedence(Precedence::ASSIGNMENT);

        if (typeid(*target) == typeid(VariableExpr)) {
            auto variableTarget = std::unique_ptr<VariableExpr>{
                    static_cast<VariableExpr *>(target.release())
            };
            return std::make_unique<AssignExpr>(std::move(variableTarget), std::move(value), oper);
        } else if (typeid(*target) == typeid(SubscriptExpr)) {
            auto subscriptTarget = std::unique_ptr<SubscriptExpr>{
                    static_cast<SubscriptExpr *>(target.release())
            };
            return std::make_unique<AllotExpr>(std::move(subscriptTarget), std::move(value), oper);
        } else if (typeid(*target) == typeid(GetExpr)) {
            auto getTarget = std::unique_ptr<GetExpr>{
                    static_cast<GetExpr *>(target.release())
            };
            return std::make_unique<SetExpr>(std::move(getTarget), std::move(value), oper);
        }

        throw errorAt(oper, "Invalid assignment target.");
    }

    std::unique_ptr<Expr> Parser::field(std::unique_ptr<Expr> object) {
        Token oper = m_previous;
        expect(TokenType::IDENTIFIER, "Expected field name after '.'.");
        Token name = m_previous;

        return std::make_unique<GetExpr>(std::move(object), name, oper);
    }

    std::unique_ptr<Expr> Parser::ternary(std::unique_ptr<Expr> condition) {
        std::unique_ptr<Expr> thenBranch = parsePrecedence(Precedence::CONDITIONAL);

        expect(TokenType::COLON, "Expected ':' after then value of conditional expression.");
        Token oper = m_previous;

        std::unique_ptr<Expr> elseBranch = parsePrecedence(Precedence::ASSIGNMENT);

        return std::make_unique<TernaryExpr>(std::move(condition), std::move(thenBranch), std::move(elseBranch), oper);
    }

    std::unique_ptr<Stmt> Parser::declaration() {
        consumeSeparator();
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

    std::unique_ptr<Stmt> Parser::functionDeclaration(bool mustParseBody, bool isMut) {
        expect(TokenType::IDENTIFIER, "Expected function name.");
        Token name = m_previous;

        expect(TokenType::LEFT_PAREN, "Expected '(' after function name.");

        std::vector<Param> params;
        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                expect(TokenType::IDENTIFIER, "Expected parameter name.");
                params.push_back(Param{m_previous, expectTypename()});
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of parameter list.");
        }

        // Get the return type
        std::unique_ptr<const Typename> returnTypename = expectTypename(true);

        std::vector<std::unique_ptr<Stmt>> body;

        if (!mustParseBody && consumeSeparator()) {
            return std::make_unique<FunctionStmt>(name, std::move(returnTypename), std::move(params), std::move(body),
                                                  nullptr, isMut);
        }

        expect(TokenType::COLON, "Expected ':' before function body.");
        consumeSeparator();

        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }

        expect(TokenType::END, "Expected 'end' at end of function declaration.");

        expectSeparator("Expected newline or ';' after function declaration.");

        return std::make_unique<FunctionStmt>(name, std::move(returnTypename), std::move(params), std::move(body),
                                              nullptr, isMut);
    }

    std::unique_ptr<Stmt> Parser::structDeclaration() {
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
        consumeSeparator();

        std::vector<Field> fields;
        std::vector<std::unique_ptr<FunctionStmt>> methods;
        std::vector<std::unique_ptr<FunctionStmt>> assocFunctions;

        while (!check(TokenType::END) && !isAtEnd()) {
            consumeSeparator();
            if (consume(TokenType::IDENTIFIER)) {
                // Field declaration
                Token fieldName = m_previous;
                std::unique_ptr<const Typename> fieldType = expectTypename();

                fields.push_back(Field{fieldName, std::move(fieldType)});

                expectSeparator("Expected newline or ';' after field declaration.");
                continue;
            }

            if (consume(TokenType::ASSOC)) {
                // Associated function declaration
                auto function = std::unique_ptr<FunctionStmt>{
                        static_cast<FunctionStmt *>(functionDeclaration().release())
                };
                assocFunctions.push_back(std::move(function));
                continue;
            }

            bool isMut = consume(TokenType::MUT);
            // We've exhausted all other possibilities, so we must consume a method declaration or else it's an error.
            expect(TokenType::FUN, "Expected field or method declaration.");

            // Parse method body
            auto method = std::unique_ptr<FunctionStmt>{
                    static_cast<FunctionStmt *>(functionDeclaration(true, isMut).release())
            };
            methods.push_back(std::move(method));
        }

        expect(TokenType::END, "Expected 'end' at end of struct declaration.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<StructStmt>(name, traits, std::move(fields), std::move(methods),
                                            std::move(assocFunctions), nullptr);
    }

    std::unique_ptr<Stmt> Parser::traitDeclaration() {
        expect(TokenType::IDENTIFIER, "Expected trait name.");
        Token name = m_previous;

        expect(TokenType::COLON, "Expected ':' after trait name.");
        consumeSeparator();

        std::vector<std::unique_ptr<FunctionStmt>> methods;
        while (!check(TokenType::END) && !isAtEnd()) {
            consumeSeparator();
            if (consume(TokenType::FUN)) {
                auto method = std::unique_ptr<FunctionStmt>{
                        static_cast<FunctionStmt *>(functionDeclaration(false).release())
                };
                methods.push_back(std::move(method));
            } else {
                throw errorAtCurrent("Expected method declaration.");
            }
        }

        expect(TokenType::END, "Expected 'end' at end of trait declaration.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<TraitStmt>(name, std::move(methods));
    }

    std::unique_ptr<Stmt> Parser::variableDeclaration(bool isConst, bool mustExpectSeparator) {
        expect(TokenType::IDENTIFIER, "Expected variable name.");
        Token name = m_previous;

        std::unique_ptr<const Typename> typeName{expectTypename(true)};

        expect(TokenType::EQUAL, "Expected '=' after variable name/type.");

        std::unique_ptr<Expr> initializer = expression();

        if (mustExpectSeparator) expectSeparator("Expected newline or ';' after variable declaration.");

        return std::make_unique<VariableStmt>(name, std::move(typeName), std::move(initializer), isConst);
    }

    std::unique_ptr<Stmt> Parser::statement() {
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

    std::unique_ptr<Stmt> Parser::blockStatement() {
        expect(TokenType::COLON, "Expected ':' before block body.");
        consumeSeparator();

        std::vector<std::unique_ptr<Stmt>> statements;
        while (!check(TokenType::END) && !isAtEnd()) {
            statements.push_back(declaration());
        }

        expect(TokenType::END, "Expected 'end' at end of block.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<BlockStmt>(std::move(statements));
    }

    std::unique_ptr<Stmt> Parser::ifStatement() {
        Token keyword = m_previous;

        std::unique_ptr<Expr> condition = expression();
        expect(TokenType::COLON, "Expected ':' after if condition.");
        consumeSeparator();

        std::vector<std::unique_ptr<Stmt>> thenBlock;
        while (!check(TokenType::END) && !check(TokenType::ELSE) && !isAtEnd()) {
            thenBlock.push_back(declaration());
        }

        std::vector<std::unique_ptr<Stmt>> elseBlock;

        if (consume(TokenType::ELSE)) {
            expect(TokenType::COLON, "Expected ':' after start of else block.");
            consumeSeparator();
            while (!check(TokenType::END) && !isAtEnd()) {
                elseBlock.push_back(declaration());
            }
        }

        expect(TokenType::END, "Expected 'end' at end of if statement.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<IfStmt>(std::move(condition), std::move(thenBlock), std::move(elseBlock), keyword);
    }

    std::unique_ptr<Stmt> Parser::whileStatement() {
        Token keyword = m_previous;

        std::unique_ptr<Expr> condition = expression();

        expect(TokenType::COLON, "Expected ':' after while condition.");
        consumeSeparator();

        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }

        expect(TokenType::END, "Expected 'end' at end of while loop.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<WhileStmt>(std::move(condition), std::move(body), keyword);
    }

    std::unique_ptr<Stmt> Parser::forStatement() {
        Token keyword = m_previous;

        std::unique_ptr<Stmt> initializer;
        if (check(TokenType::SEPARATOR)) {
            initializer = std::make_unique<ExpressionStmt>(std::make_unique<NilExpr>());
        } else if (consume(TokenType::VAR)) {
            initializer = variableDeclaration(false, false);
        } else if (consume(TokenType::CONST)) {
            initializer = variableDeclaration(true, false);
        } else {
            initializer = std::make_unique<ExpressionStmt>(expression());
        }

        expect(TokenType::SEPARATOR, "Expected '|' after for loop initializer.");

        std::unique_ptr<Expr> condition;
        if (!check(TokenType::SEPARATOR)) {
            condition = expression();
        } else {
            condition = std::make_unique<BooleanExpr>(true);
        }

        expect(TokenType::SEPARATOR, "Expected '|' after for loop condition.");

        std::unique_ptr<Expr> increment;
        if (!check(TokenType::COLON)) {
            increment = expression();
        } else {
            increment = std::make_unique<NilExpr>();
        }

        expect(TokenType::COLON, "Expected ':' before body of for loop.");

        consumeSeparator();

        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }

        expect(TokenType::END, "Expected 'end' at end of for loop.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment),
                                         std::move(body), keyword);
    }

    std::unique_ptr<Stmt> Parser::eachStatement() {
        expect(TokenType::IDENTIFIER, "Expected item name after 'each'.");
        Token name = m_previous;

        expect(TokenType::IN, "Expected 'in' after each loop item name.");

        std::unique_ptr<Expr> object = expression();

        expect(TokenType::COLON, "Expected ':' before each loop body.");
        consumeSeparator();

        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::END) && !isAtEnd()) {
            body.push_back(declaration());
        }

        expect(TokenType::END, "Expected 'end' at end of each loop.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<EachStmt>(name, std::move(object), std::move(body));
    }

    std::unique_ptr<Stmt> Parser::givenStatement() {
        std::unique_ptr<Expr> value = expression();
        expect(TokenType::COLON, "Expected ':' before given statement body.");

        std::vector<GivenCase> cases;
        while (!check(TokenType::END) && !isAtEnd()) {
            consumeSeparator();

            if (consume(TokenType::WHEN)) {
                Token keyword = m_previous;

                std::unique_ptr<Expr> caseValue = expression();
                expect(TokenType::COLON, "Expected ':' before case body");

                std::vector<std::unique_ptr<Stmt>> caseBody;
                while (!check(TokenType::WHEN) && !check(TokenType::ELSE) && !check(TokenType::END) && !isAtEnd()) {
                    caseBody.push_back(declaration());
                }

                cases.push_back(GivenCase{std::move(caseValue), std::move(caseBody), keyword});
            } else if (consume(TokenType::ELSE)) {
                Token keyword = m_previous;

                expect(TokenType::COLON, "Expected ':' before 'else' case body.");

                std::vector<std::unique_ptr<Stmt>> caseBody;
                while (!check(TokenType::WHEN) && !check(TokenType::END) && !isAtEnd()) {
                    caseBody.push_back(declaration());
                }

                std::unique_ptr<Expr> caseValue = std::make_unique<AnyExpr>();

                cases.push_back(GivenCase{std::move(caseValue), std::move(caseBody), keyword});
            }
        }

        expect(TokenType::END, "Expected 'end' at end of given statement.");
        expectSeparator("Expected newline or ';' after 'end'.");

        return std::make_unique<GivenStmt>(std::move(value), std::move(cases));
    }

    std::unique_ptr<Stmt> Parser::returnStatement() {
        Token keyword = m_previous;
        std::unique_ptr<Expr> value = expression();

        expectSeparator("Expected newline or ';' after return statement.");

        return std::make_unique<ReturnStmt>(keyword, std::move(value));
    }

    std::unique_ptr<Stmt> Parser::breakStatement() {
        expectSeparator("Expected newline or ';' after break statement.");
        return std::make_unique<BreakStmt>(m_previous);
    }

    std::unique_ptr<Stmt> Parser::continueStatement() {
        expectSeparator("Expected newline or ';' after continue statement.");
        return std::make_unique<ContinueStmt>(m_previous);
    }

    std::unique_ptr<Stmt> Parser::expressionStatement() {
        std::unique_ptr<Expr> expr = expression();
        expectSeparator("Expected newline or ';' after expression.");
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::vector<std::unique_ptr<Stmt>> Parser::parse() {
        m_scanner = Lexer{m_context.source};
        advance();

        std::vector<std::unique_ptr<Stmt>> ast{};
        while (!isAtEnd()) {
            std::unique_ptr<Stmt> stmt = declaration();
            if (stmt) ast.push_back(std::move(stmt));
        }

        return ast;
    }

    Parser::ParseError Parser::errorAt(const Token &token, const std::string &message) {
        m_context.reportErrorAt(token, message);
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

            m_context.reportErrorAt(m_current, m_current.lexeme);
        }
    }

    void Parser::undoAdvance() {
        m_current = m_previous;
        m_previous = m_scanner.backtrack();
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
            throw errorAtCurrent(message);
        }
    }

    bool Parser::consumeSeparator() {
        bool didConsume = false;
        while (consume(TokenType::NEWLINE) || consume(TokenType::SEMICOLON)) {
            didConsume = true;
        }
        return didConsume;
    }

    void Parser::expectSeparator(const std::string &message) {
        if (consumeSeparator()) return;
        throw errorAtCurrent(message);
    }

    std::unique_ptr<const Typename> Parser::expectTypename(bool emptyAllowed) {
        std::unique_ptr<const Typename> typeName;

        bool isEnclosed = consume(TokenType::LEFT_PAREN);

        if (consume(TokenType::STRUCT)) {
            expect(TokenType::IDENTIFIER, "Expected struct name to complete constructor typename.");
            return std::make_unique<ConstructorTypename>(std::make_unique<BasicTypename>(m_previous));
        }

        if (consume(TokenType::FUN)) {
            typeName = expectFunctionTypename();
        } else if (consume(TokenType::IDENTIFIER)) {
            typeName = std::make_unique<BasicTypename>(m_previous);
        } else if (!emptyAllowed) {
            throw error("Expected a typename.");
        }

        if (typeName && consume(TokenType::LEFT_SQUARE)) {
            expect(TokenType::RIGHT_SQUARE, "Expected ']' to complete list typename.");
            typeName = std::make_unique<ArrayTypename>(std::move(typeName));
        }

        if (isEnclosed && !consume(TokenType::RIGHT_PAREN)) {
            throw error("Expected ')' after typename.");
        }

        if (!typeName && emptyAllowed) {
            typeName = std::make_unique<BasicTypename>("", m_previous);
        }

        return typeName;
    }

    std::unique_ptr<const Typename> Parser::expectFunctionTypename() {
        expect(TokenType::LEFT_PAREN, "Expected '(' after 'fun' in function type.");

        std::vector<std::unique_ptr<const Typename>> argumentTypenames;
        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                argumentTypenames.push_back(expectTypename());
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of parameter list in function type.");
        }

        std::unique_ptr<const Typename> returnTypename = expectTypename(true);

        return std::make_unique<FunctionTypename>(std::move(returnTypename), std::move(argumentTypenames));
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
}