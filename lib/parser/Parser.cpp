#include "../context/CompileContext.h"

#include "Token.h"

namespace enact {
    Parser::Parser(CompileContext &context) : m_context{context} {
    }

    std::vector<std::unique_ptr<Stmt>> Parser::parse() {
        m_scanner = Lexer{m_context.getSource()};
        advance();

        std::vector<std::unique_ptr<Stmt>> ast{};
        while (!isAtEnd()) {
            std::unique_ptr<Stmt> stmt = declaration();
            if (stmt) ast.push_back(std::move(stmt));
        }

        return ast;
    }

    bool Parser::hadError() const {
        return m_hadError;
    }

    std::unique_ptr<Stmt> Parser::parseStmt() {
        try {
            if (consume(TokenType::FUNC))     return parseFunctionStmt();
            if (consume(TokenType::STRUCT))   return parseStructStmt();
            if (consume(TokenType::ENUM))     return parseEnumStmt();
            if (consume(TokenType::TRAIT))    return parseTraitStmt();
            if (consume(TokenType::IMPL))     return parseImplStmt();
            if (consume(TokenType::IMM) ||
                consume(TokenType::MUT))      return parseVariableStmt();
            if (consume(TokenType::RETURN))   return parseReturnStmt();
            if (consume(TokenType::BREAK))    return parseBreakStmt();
            if (consume(TokenType::CONTINUE)) return parseContinueStmt();
            return parseExpressionStmt();
        } catch (ParseError &error) {
            synchronise();
            return nullptr;
        }
    }

    std::unique_ptr<Stmt> Parser::parseFunctionStmt(bool mustParseBody) {
        expect(TokenType::IDENTIFIER, "Expected function name.");
        Token name = m_previous;

        expect(TokenType::LEFT_PAREN, "Expected '(' after function name.");

        std::vector<FunctionStmt::Param> params;
        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                expect(TokenType::IDENTIFIER, "Expected parameter name.");
                params.push_back(FunctionStmt::Param{m_previous, expectTypename()});
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of parameter list.");
        }

        // Get the return type
        std::unique_ptr<const Typename> returnTypename = expectTypename(true);

        if (!mustParseBody && consume(TokenType::SEMICOLON)) {
            return std::make_unique<FunctionStmt>(name,
                                                  std::move(returnTypename),
                                                  std::move(params),
                                                  std::make_unique<BlockExpr>(std::vector<std::unique_ptr<Stmt>>{},
                                                                              std::make_unique<UnitExpr>(m_previous)));
        }

        std::unique_ptr<BlockExpr> body = static_unique_ptr_cast<BlockExpr>(parseBlockExpr());
        return std::make_unique<FunctionStmt>(std::move(name), std::move(returnTypename), std::move(params), std::move(body));
    }

    std::unique_ptr<Stmt> Parser::parseStructStmt() {
        expect(TokenType::IDENTIFIER, "Expected struct name.");
        Token name = m_previous;

        expect(TokenType::LEFT_BRACE, "Expected '{' before struct body.");

        std::vector<StructStmt::Field> fields;

        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            expect(TokenType::IDENTIFIER, "Expected field declaration in struct body.");
            // Field declaration
            Token fieldName = m_previous;
            std::unique_ptr<const Typename> fieldType = expectTypename();

            fields.push_back(StructStmt::Field{std::move(fieldName), std::move(fieldType)});

            expect(TokenType::SEMICOLON, "Expected ';' after struct field declaration.");
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after struct body.");

        return std::make_unique<StructStmt>(name, std::move(fields));
    }

    std::unique_ptr<Stmt> Parser::parseEnumStmt() {
        expect(TokenType::IDENTIFIER, "Expected enum name.");
        Token name = m_previous;

        expect(TokenType::LEFT_BRACE, "Expected '{' before enum body.");

        std::vector<EnumStmt::Variant> variants;

        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            expect(TokenType::IDENTIFIER, "Expected variant declaration in struct body.");
            // Field declaration
            Token variantName = m_previous;
            std::unique_ptr<const Typename> variantType = expectTypename("Expected typename after variant name.", true);

            variants.push_back(EnumStmt::Variant{std::move(variantName), std::move(variantType)});

            expect(TokenType::SEMICOLON, "Expected ';' after enum variant declaration.");
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after enum body.");

        return std::make_unique<EnumStmt>(std::move(name), std::move(variants));
    }

    std::unique_ptr<Stmt> Parser::parseTraitStmt() {
        expect(TokenType::IDENTIFIER, "Expected trait name.");
        Token name = m_previous;

        expect(TokenType::LEFT_BRACE, "Expected '{' before trait body.");

        std::vector<std::unique_ptr<FunctionStmt>> methods;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            if (consume(TokenType::FUNC)) {
                methods.push_back(
                        static_unique_ptr_cast<FunctionStmt>(
                                parseFunctionStmt(false)));
            } else {
                throw errorAtCurrent("Expected method declaration in trait body.");
            }
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after trait body.");

        return std::make_unique<TraitStmt>(name, std::move(methods));
    }

    std::unique_ptr<Stmt> Parser::parseImplStmt() {
        std::unique_ptr<const Typename> firstTypename = expectTypename("Expected typename after 'impl'.");
        std::unique_ptr<const Typename> secondTypename = consume(TokenType::FOR)
                                                         ? expectTypename("Expected typename after 'impl'..'for'.")
                                                         : nullptr;

        expect(TokenType::LEFT_BRACE, "Expected '{' before impl body.");

        std::vector<std::unique_ptr<FunctionStmt>> methods;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            if (consume(TokenType::FUNC)) {
                methods.push_back(
                        static_unique_ptr_cast<FunctionStmt>(
                                parseFunctionStmt()));
            } else {
                throw errorAtCurrent("Expected method declaration in impl body.");
            }
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after impl body.");

        if (secondTypename == nullptr) {
            // Inherent impl
            return std::make_unique<ImplStmt>(std::move(firstTypename), std::move(secondTypename), std::move(methods));
        }
        // Trait impl
        return std::make_unique<ImplStmt>(std::move(secondTypename), std::move(firstTypename), std::move(methods));
    }

    std::unique_ptr<Stmt> Parser::parseVariableStmt() {
        Token keyword = m_previous;
        expect(TokenType::IDENTIFIER, "Expected variable name.");
        Token name = m_previous;

        std::unique_ptr<const Typename> typeName{
                expectTypename("Expected typename after variable name.", true)};

        expect(TokenType::EQUAL, "Expected '=' after variable name/type.");

        std::unique_ptr<Expr> initializer = parseExpr();

        expect(TokenType::SEMICOLON, "Expected ';' after variable declaration.");

        return std::make_unique<VariableStmt>(
                std::move(keyword),
                std::move(name),
                std::move(typeName),
                std::move(initializer));
    }

    std::unique_ptr<Stmt> Parser::parseReturnStmt() {
        Token keyword = m_previous;
        std::unique_ptr<Expr> value;
        if (check(TokenType::SEMICOLON)) {
            value = std::make_unique<UnitExpr>(m_current);
        } else {
            value = parseExpr();
        }

        expect(TokenType::SEMICOLON, "Expected ';' after return statement.");
        return std::make_unique<ReturnStmt>(keyword, std::move(value));
    }

    std::unique_ptr<Stmt> Parser::parseBreakStmt() {
        Token keyword = m_previous;
        std::unique_ptr<Expr> value;
        if (check(TokenType::SEMICOLON)) {
            value = std::make_unique<UnitExpr>(m_current);
        } else {
            value = parseExpr();
        }

        expect(TokenType::SEMICOLON, "Expected ';' after break statement.");
        return std::make_unique<BreakStmt>(std::move(keyword), std::move(value));
    }

    std::unique_ptr<Stmt> Parser::parseContinueStmt() {
        expect(TokenType::SEMICOLON, "Expected ';' after continue statement.");
        return std::make_unique<ContinueStmt>(m_previous);
    }

    std::unique_ptr<Stmt> Parser::parseExpressionStmt() {
        std::unique_ptr<Expr> expr = parseExpr();
        if (m_previous.type != TokenType::RIGHT_BRACE) {
            expect(TokenType::SEMICOLON, "Expected ';' after expression statement.");
        }
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::unique_ptr<Expr> Parser::parseExpr() {
        // Expression with block?
        if (consume(TokenType::RIGHT_BRACE) ||
            consume(TokenType::EQUAL_GREATER)) return parseBlockExpr();
        if (consume(TokenType::IF))            return parseIfExpr();
        if (consume(TokenType::WHILE))         return parseWhileExpr();
        if (consume(TokenType::FOR))           return parseForExpr();
        if (consume(TokenType::SWITCH))        return parseSwitchExpr();

        // Expression without block. Start at the lowest precedence.
        return parsePrecAssignment();
    }

    std::unique_ptr<Expr> Parser::parsePrecAssignment() {
        std::unique_ptr<Expr> expr = parsePrecLogicalOr();

        if (consume(TokenType::EQUAL)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecAssignment(); // Right-associative
            return std::make_unique<AssignExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecLogicalOr() {
        std::unique_ptr<Expr> expr = parsePrecLogicalAnd();

        while (consume(TokenType::OR)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecLogicalAnd();
            expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecLogicalAnd() {
        std::unique_ptr<Expr> expr = parsePrecEquality();

        while (consume(TokenType::AND)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecEquality();
            expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecEquality() {
        std::unique_ptr<Expr> expr = parsePrecComparison();

        while (consume(TokenType::EQUAL_EQUAL) ||
               consume(TokenType::BANG_EQUAL)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecComparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecComparison() {
        std::unique_ptr<Expr> expr = parsePrecCast();

        while (consume(TokenType::LESS) ||
               consume(TokenType::LESS_EQUAL) ||
               consume(TokenType::GREATER) ||
               consume(TokenType::GREATER_EQUAL)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecCast();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecBitwiseOr() {
        std::unique_ptr<Expr> expr = parsePrecBitwiseXor();

        while (consume(TokenType::PIPE)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecBitwiseXor();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecBitwiseXor() {
        std::unique_ptr<Expr> expr = parsePrecBitwiseAnd();

        while (consume(TokenType::CARAT)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecBitwiseAnd();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecBitwiseAnd() {
        std::unique_ptr<Expr> expr = parsePrecAdd();

        while (consume(TokenType::AMPERSAND)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecAdd();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecAdd() {
        std::unique_ptr<Expr> expr = parsePrecMultiply();

        while (consume(TokenType::PLUS) ||
               consume(TokenType::MINUS)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecMultiply();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecMultiply() {
        std::unique_ptr<Expr> expr = parsePrecBitwiseShift();

        while (consume(TokenType::STAR) ||
               consume(TokenType::SLASH)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecBitwiseShift();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parseGroupingExpr() {
        std::unique_ptr<Expr> expr = parseExpr();
        expect(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
        return expr;
    }

    std::unique_ptr<Expr> Parser::parseLiteralExpr() {
        switch (m_previous.type) {
            case TokenType::INTEGER:
                return std::make_unique<IntegerExpr>(std::stoi(m_previous.lexeme));
            case TokenType::FLOAT:
                return std::make_unique<FloatExpr>(std::stod(m_previous.lexeme));
            case TokenType::TRUE:
                return std::make_unique<BooleanExpr>(true);
            case TokenType::FALSE:
                return std::make_unique<BooleanExpr>(false);
            case TokenType::STRING:
                return std::make_unique<StringExpr>(std::move(m_previous.lexeme));
            case TokenType::IDENTIFIER:
                return std::make_unique<SymbolExpr>(m_previous);
            default:
                ENACT_UNREACHABLE();
        }
    }

    std::unique_ptr<Expr> Parser::parseUnaryExpr() {
        Token oper = m_previous;

        std::unique_ptr<Expr> expr = parseAtPrecedence(Precedence::UNARY);

        return std::make_unique<UnaryExpr>(std::move(expr), std::move(oper));
    }

    std::unique_ptr<Expr> Parser::parseCallExpr(std::unique_ptr<Expr> callee) {
        Token leftParen = m_previous;
        std::vector<std::unique_ptr<Expr>> arguments;

        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                arguments.push_back(parseExpr());
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of argument list.");
        }

        if (arguments.size() > 255) {
            throw errorAt(leftParen,
                    std::to_string(arguments.size()) +
                    " arguments in function call; maximum is 255.");
        }

        return std::make_unique<CallExpr>(std::move(callee), std::move(arguments), std::move(leftParen));
    }

    std::unique_ptr<Expr> Parser::parseBinaryExpr(std::unique_ptr<Expr> left) {
        Token oper = m_previous;

        const ParseRule& rule = getParseRule(oper.type);
        std::unique_ptr<Expr> right = parseAtPrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1)));

        switch (oper.type) {
            case TokenType::EQUAL:
                return std::make_unique<AssignExpr>(std::move(left), std::move(right), std::move(oper));

            case TokenType::AND:
            case TokenType::OR:
                return std::make_unique<LogicalExpr>(std::move(left), std::move(right), std::move(oper));

            default:
                return std::make_unique<BinaryExpr>(std::move(left), std::move(right), std::move(oper));
        }
    }

    std::unique_ptr<Expr> Parser::parseFieldExpr(std::unique_ptr<Expr> object) {
        Token oper = m_previous;
        expect(TokenType::IDENTIFIER, "Expected field name after '.'.");
        Token name = m_previous;

        return std::make_unique<FieldExpr>(std::move(object), std::move(name), std::move(oper));
    }

    std::unique_ptr<Expr> Parser::parseBlockExpr() {
        // Single-expression block?
        if (m_previous.type == TokenType::EQUAL_GREATER) {
            return std::make_unique<BlockExpr>(std::vector<std::unique_ptr<Stmt>>{}, parseExpr());
        }

        std::vector<std::unique_ptr<Stmt>> stmts;
        do {
            stmts.push_back(parseDeclarationStmt());
        } while (m_previous.type == TokenType::SEMICOLON && !isAtEnd());

        if (check(TokenType::RIGHT_BRACE)) {
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            stmts.push_back(parseDeclarationStmt());
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after block body.");

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



    ParseError Parser::errorAt(const Token &token, const std::string &message) {
        m_context.reportErrorAt(token, message);
        m_hadError = true;
        return ParseError{};
    }

    ParseError Parser::errorAtCurrent(const std::string &message) {
        return errorAt(m_current, message);
    }

    ParseError Parser::error(const std::string &message) {
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

    std::unique_ptr<const Typename> Parser::expectTypename(const std::string& msg, bool emptyAllowed) {
        std::unique_ptr<const Typename> typeName;

        bool isEnclosed = consume(TokenType::LEFT_PAREN);

        if (consume(TokenType::STRUCT)) {
            expect(TokenType::IDENTIFIER, "Expected struct name to complete constructor typename.");
            return std::make_unique<ConstructorTypename>(std::make_unique<BasicTypename>(m_previous));
        }

        if (consume(TokenType::FUNC)) {
            typeName = expectFunctionTypename(msg);
        } else if (consume(TokenType::IDENTIFIER)) {
            typeName = std::make_unique<BasicTypename>(m_previous);
        } else if (!emptyAllowed) {
            throw error(msg);
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

    std::unique_ptr<const Typename> Parser::expectFunctionTypename(const std::string& msg) {
        expect(TokenType::LEFT_PAREN, "Expected '(' after 'fun' in function type.");

        std::vector<std::unique_ptr<const Typename>> argumentTypenames;
        if (!consume(TokenType::RIGHT_PAREN)) {
            do {
                argumentTypenames.push_back(expectTypename("Expected parameter type in function type parameter list."));
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
                case TokenType::BREAK:
                case TokenType::CONTINUE:
                case TokenType::ENUM:
                case TokenType::FOR:
                case TokenType::FUNC:
                case TokenType::IF:
                case TokenType::IMM:
                case TokenType::IMPL:
                case TokenType::MUT:
                case TokenType::RETURN:
                case TokenType::SEMICOLON:
                case TokenType::STRUCT:
                case TokenType::TRAIT:
                case TokenType::WHILE:
                    return;
            }

            advance();
        }
    }

    bool Parser::isAtEnd() {
        return m_current.type == TokenType::END_OF_FILE;
    }

    const ParseRule &Parser::getParseRule(TokenType type) {
        return m_parseRules[static_cast<std::size_t>(type)];
    }
}