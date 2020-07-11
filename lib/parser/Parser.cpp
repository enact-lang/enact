#include <enact/parser/Parser.h>

#include <enact/context/CompileContext.h>
#include <enact/parser/Token.h>

namespace enact {
    Parser::Parser(CompileContext &context) : m_context{context} {
    }

    std::vector<std::unique_ptr<Stmt>> Parser::parse() {
        m_scanner = Lexer{m_context.getSource()};
        advance();

        std::vector<std::unique_ptr<Stmt>> ast{};
        while (!isAtEnd()) {
            std::unique_ptr<Stmt> stmt = parseStmt();
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
                params.push_back(FunctionStmt::Param{m_previous, expectTypename("Expected parameter typename.")});
            } while (consume(TokenType::COMMA));

            expect(TokenType::RIGHT_PAREN, "Expected end of parameter list.");
        }

        // Get the return type
        // At second-lowest precedence to force grouping of function types (because of => ambiguity)
        std::unique_ptr<const Typename> returnTypename =
                expectTypenamePrecUnary("Expected return typename.", true);

        if (!mustParseBody && check(TokenType::SEMICOLON)) {
            return std::make_unique<FunctionStmt>(name,
                                                  std::move(returnTypename),
                                                  std::move(params),
                                                  std::make_unique<BlockExpr>(std::vector<std::unique_ptr<Stmt>>{},
                                                                              std::make_unique<UnitExpr>(m_previous)));
        }

        std::unique_ptr<BlockExpr> body = expectBlock("Expected '{' or '=>' before function body.");
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
            std::unique_ptr<const Typename> fieldType = expectTypename("Expected field typename.");

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

        return std::make_unique<BreakStmt>(std::move(keyword), std::move(value));
    }

    std::unique_ptr<Stmt> Parser::parseContinueStmt() {
        return std::make_unique<ContinueStmt>(m_previous);
    }

    std::unique_ptr<Stmt> Parser::parseExpressionStmt() {
        std::unique_ptr<Expr> expr = parseExpr();
        return std::make_unique<ExpressionStmt>(std::move(expr));
    }

    std::unique_ptr<Expr> Parser::parseExpr() {
        // Expression with block?
        if (consume(TokenType::LEFT_BRACE) ||
            consume(TokenType::EQUAL_GREATER)) return parseBlockExpr();
        if (consume(TokenType::IF))            return parseIfExpr();
        if (consume(TokenType::WHILE))         return parseWhileExpr();
        if (consume(TokenType::FOR))           return parseForExpr();
        if (consume(TokenType::SWITCH))        return parseSwitchExpr();

        // Expression without block. Start at the lowest precedence.
        return parsePrecAssignment();
    }

    std::unique_ptr<Expr> Parser::parseBlockExpr() {
        Token start = m_previous;

        std::vector<std::unique_ptr<Stmt>> body{};
        std::unique_ptr<Expr> end{std::make_unique<UnitExpr>(m_previous)};

        if (start.type == TokenType::EQUAL_GREATER) {
            end = parseExpr();
        } else if (start.type == TokenType::LEFT_BRACE) {
            while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
                std::unique_ptr<Stmt> current = parseStmt();

                if (consume(TokenType::SEMICOLON)) {
                    body.push_back(std::move(current));
                    if (check(TokenType::RIGHT_BRACE)) break;
                } else {
                    if (auto exprStmt = dynamic_unique_ptr_cast<ExpressionStmt>(std::move(current))) {
                        end = std::move(exprStmt->expr);
                    } else {
                        throw error("Expected expression or ';' at end of block.");
                    }
                    break;
                }
            }

            expect(TokenType::RIGHT_BRACE, "Expected '}' at end of block.");
        }

        return std::make_unique<BlockExpr>(std::move(body), std::move(end));
    }

    std::unique_ptr<Expr> Parser::parseIfExpr() {
        Token keyword = m_previous;

        std::unique_ptr<Expr> condition = parseExpr();
        std::unique_ptr<BlockExpr> thenBlock = expectBlock("Expected '{' or '=>' before if expression body.");

        std::unique_ptr<BlockExpr> elseBlock = std::make_unique<BlockExpr>(
                std::vector<std::unique_ptr<Stmt>>{},
                std::make_unique<UnitExpr>(m_previous));

        if (consume(TokenType::ELSE)) {
            // Special case, so we can type 'else if' instead of 'else => if'
            if (consume(TokenType::IF)) {
                elseBlock->expr = parseIfExpr();
            } else {
                elseBlock = expectBlock("Expected '{' or '=>' after 'else'.");
            }
        }

        return std::make_unique<IfExpr>(
                std::move(condition),
                std::move(thenBlock),
                std::move(elseBlock),
                std::move(keyword));
    }

    std::unique_ptr<Expr> Parser::parseWhileExpr() {
        Token keyword = m_previous;

        std::unique_ptr<Expr> condition = parseExpr();
        std::unique_ptr<BlockExpr> body = expectBlock("Expected '{' or '=>' before while loop body.");

        return std::make_unique<WhileExpr>(std::move(condition), std::move(body), std::move(keyword));
    }

    std::unique_ptr<Expr> Parser::parseForExpr() {
        expect(TokenType::IDENTIFIER, "Expected item name after 'for'.");
        Token name = m_previous;

        expect(TokenType::IN, "Expected 'in' after for loop item name.");

        std::unique_ptr<Expr> iterator = parseExpr();
        std::unique_ptr<BlockExpr> body = expectBlock("Expected '{' or '=>' before for loop body.");

        return std::make_unique<ForExpr>(std::move(name), std::move(iterator), std::move(body));
    }

    std::unique_ptr<Expr> Parser::parseSwitchExpr() {
        std::unique_ptr<Expr> value = parseExpr();
        expect(TokenType::LEFT_BRACE, "Expected '{' before switch expression body.");

        std::vector<SwitchCase> cases;
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            if (consume(TokenType::CASE)) {
                Token keyword = m_previous;

                // TODO: add more pattern types
                std::unique_ptr<Pattern> casePattern = std::make_unique<ValuePattern>(parseExpr());
                std::unique_ptr<Expr> casePredicate = consume(TokenType::WHEN)
                                                    ? parseExpr()
                                                    : std::make_unique<BooleanExpr>(true);
                std::unique_ptr<BlockExpr> caseBody = expectBlock("Expected '{' or '=>' before switch case body.");

                cases.push_back(SwitchCase{std::move(casePattern), std::move(casePredicate), std::move(caseBody), std::move(keyword)});
            } else if (consume(TokenType::DEFAULT)) {
                Token keyword = m_previous;

                std::unique_ptr<Pattern> casePattern = std::make_unique<WildcardPattern>(keyword);
                std::unique_ptr<Expr> casePredicate = consume(TokenType::WHEN)
                                                      ? parseExpr()
                                                      : std::make_unique<BooleanExpr>(true);
                std::unique_ptr<BlockExpr> caseBody = expectBlock("Expected '{' or '=>' before default switch case body.");

                cases.push_back(SwitchCase{std::move(casePattern),
                                           std::move(casePredicate),
                                           std::move(caseBody),
                                           std::move(keyword)});
            }
        }

        expect(TokenType::RIGHT_BRACE, "Expected '}' after switch expression body.");

        return std::make_unique<SwitchExpr>(std::move(value), std::move(cases));
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

    std::unique_ptr<Expr> Parser::parsePrecCast() {
        std::unique_ptr<Expr> expr = parsePrecRange();

        if (consume(TokenType::AS) || consume(TokenType::IS)) {
            Token oper = m_previous;
            std::unique_ptr<const Typename> typename_ = expectTypename(
                    "Expected typename after '" + m_previous.lexeme + "'.");
            expr = std::make_unique<CastExpr>(std::move(expr), std::move(typename_), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecRange() {
        std::unique_ptr<Expr> expr = parsePrecBitwiseOr();

        while (consume(TokenType::DOT_DOT) || consume(TokenType::DOT_DOT_DOT)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecBitwiseOr();
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

    std::unique_ptr<Expr> Parser::parsePrecBitwiseShift() {
        std::unique_ptr<Expr> expr = parsePrecUnary();

        while (consume(TokenType::LESS_LESS) ||
               consume(TokenType::GREATER_GREATER)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> rightExpr = parsePrecUnary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(rightExpr), std::move(oper));
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecUnary() {
        if (consume(TokenType::MINUS) ||
            consume(TokenType::TILDE) ||
            consume(TokenType::STAR) ||
            consume(TokenType::NOT)) {
            Token oper = m_previous;
            std::unique_ptr<Expr> expr = parsePrecUnary(); // Right recursion
            return std::make_unique<UnaryExpr>(std::move(expr), std::move(oper));
        }

        if (consume(TokenType::AMPERSAND)) {
            Token oper = m_previous;

            std::optional<Token> permission;
            if (consume(TokenType::MUT) || consume(TokenType::IMM)) {
                permission = m_previous;
            }

            std::optional<Token> region;
            if (consume(TokenType::SO) || consume(TokenType::RC) || consume(TokenType::GC)) {
                region = m_previous;
            }

            std::unique_ptr<Expr> expr = parsePrecUnary(); // Right recursion

            return std::make_unique<ReferenceExpr>(
                    std::move(expr),
                    std::move(oper),
                    std::move(permission),
                    std::move(region));
        }

        return parsePrecCall();
    }

    std::unique_ptr<Expr> Parser::parsePrecCall() {
        std::unique_ptr<Expr> expr = parsePrecPrimary();

        while (true) {
            if (consume(TokenType::LEFT_PAREN)) {
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RIGHT_PAREN)) {
                    do {
                        args.push_back(parseExpr());
                    } while (consume(TokenType::COMMA));
                }

                expect(TokenType::RIGHT_PAREN, "Expected ')' after function call arguments.");
                Token paren = m_previous;

                if (args.size() > 255) {
                    throw errorAt(paren,
                            "Too many arguments in function call; " +
                            std::to_string(args.size()) +
                            " were provided, max is 255.");
                }

                expr = std::make_unique<CallExpr>(std::move(expr), std::move(args), std::move(paren));
            } else if (consume(TokenType::DOT)) {
                Token dot = m_previous;

                expect(TokenType::IDENTIFIER, "Expected property name after '.'.");
                Token name = m_previous;

                expr = std::make_unique<FieldExpr>(std::move(expr), std::move(name), std::move(dot));
            } else {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<Expr> Parser::parsePrecPrimary() {
        if (consume(TokenType::INTEGER)) {
            return std::make_unique<IntegerExpr>(std::stoi(m_previous.lexeme));
        }
        if (consume(TokenType::FLOAT)) {
            return std::make_unique<FloatExpr>(std::stod(m_previous.lexeme));
        }

        if (consume(TokenType::TRUE))  {
            return std::make_unique<BooleanExpr>(true);
        }
        if (consume(TokenType::FALSE)) {
            return std::make_unique<BooleanExpr>(false);
        }

        if (consume(TokenType::STRING)) {
            return std::make_unique<StringExpr>(m_previous.lexeme);
        }

        if (consume(TokenType::INTERPOLATION)) {
            return parseInterpolationExpr();
        }

        if (consume(TokenType::IDENTIFIER)) {
            return std::make_unique<SymbolExpr>(m_previous);
        }

        if (consume(TokenType::LEFT_PAREN)) {
            Token paren = m_previous;

            // Unit type ()
            if (consume(TokenType::RIGHT_PAREN)) return std::make_unique<UnitExpr>(std::move(paren));

            // Grouping (expr)
            std::unique_ptr<Expr> expr = parseExpr();
            if (consume(TokenType::RIGHT_PAREN)) return expr;

            // Tuple (expr, expr...)
            std::vector<std::unique_ptr<Expr>> elems{};
            elems.push_back(std::move(expr));
            while (consume(TokenType::COMMA)) {
                elems.push_back(parseExpr());
            }

            expect(TokenType::RIGHT_PAREN, "Expected ')' after tuple elements.");

            return std::make_unique<TupleExpr>(std::move(elems), std::move(paren));
        }

        throw errorAtCurrent("Expected expression.");
    }

    std::unique_ptr<Expr> Parser::parseInterpolationExpr() {
        Token token = m_previous;
        std::unique_ptr<StringExpr> start = std::make_unique<StringExpr>(m_previous.lexeme);
        std::unique_ptr<Expr> interpolated = parseExpr();
        std::unique_ptr<Expr> end;

        if (consume(TokenType::INTERPOLATION)) {
            end = parseInterpolationExpr();
        } else {
            expect(TokenType::STRING, "Expected end of string interpolation");
            end = std::make_unique<StringExpr>(m_previous.lexeme);
        }

        return std::make_unique<InterpolationExpr>(
                std::move(start),
                std::move(interpolated),
                std::move(end),
                std::move(token));
    }

    std::unique_ptr<const Typename> Parser::expectTypename(const std::string& msg, bool emptyAllowed) {
        return expectTypenamePrecFunction(msg, emptyAllowed);
    }

    std::unique_ptr<const Typename> Parser::expectTypenamePrecFunction(const std::string& msg, bool emptyAllowed) {
        std::unique_ptr<const Typename> typename_ = expectTypenamePrecUnary(msg, emptyAllowed);

        if (consume(TokenType::EQUAL_GREATER)) {
            std::vector<std::unique_ptr<const Typename>> parameterTypenames;
            if (typename_->kind() == Typename::Kind::TUPLE) {
                parameterTypenames = cloneAll(
                        static_unique_ptr_cast<const TupleTypename>(
                                std::move(typename_))
                                ->elementTypenames());
            } else {
                parameterTypenames.push_back(std::move(typename_));
            }

            std::unique_ptr<const Typename> returnTypename =
                    expectTypename("Expected return typename after '=>'.");

            typename_ = std::make_unique<FunctionTypename>(std::move(returnTypename), std::move(parameterTypenames));
        }

        return typename_;
    }

    std::unique_ptr<const Typename> Parser::expectTypenamePrecUnary(const std::string &msg, bool emptyAllowed) {
        if (consume(TokenType::AMPERSAND)) {
            std::optional<Token> permission;
            if (consume(TokenType::MUT) || consume(TokenType::IMM)) {
                permission = m_previous;
            }

            std::optional<Token> region;
            if (consume(TokenType::SO) || consume(TokenType::RC) || consume(TokenType::GC)) {
                region = m_previous;
            }

            std::unique_ptr<const Typename> referringTypename = expectTypenamePrecUnary("Expected typename after '&'.");

            return std::make_unique<ReferenceTypename>(
                    std::move(permission),
                    std::move(region),
                    std::move(referringTypename));
        }
        if (consume(TokenType::QUESTION)) {
            return std::make_unique<OptionalTypename>(expectTypenamePrecUnary("Expected typename after '?'."));
        }

        return expectTypenamePrecParametric(msg, emptyAllowed);
    }

    std::unique_ptr<const Typename> Parser::expectTypenamePrecParametric(const std::string &msg, bool emptyAllowed) {
        std::unique_ptr<const Typename> typename_ = expectTypenamePrecPrimary(msg, emptyAllowed);

        if (consume(TokenType::LEFT_SQUARE)) {
            std::vector<std::unique_ptr<const Typename>> parameterTypenames;

            do {
                parameterTypenames.push_back(expectTypename("Expected typename parameters after '['."));
            } while (consume(TokenType::COMMA));
            expect(TokenType::RIGHT_SQUARE, "Expected ']' after typename parameters.");

            typename_ = std::make_unique<ParametricTypename>(std::move(typename_), std::move(parameterTypenames));
        }

        return typename_;
    }

    std::unique_ptr<const Typename> Parser::expectTypenamePrecPrimary(const std::string &msg, bool emptyAllowed) {
        if (consume(TokenType::IDENTIFIER)) {
            return std::make_unique<BasicTypename>(m_previous);
        }

        if (consume(TokenType::DOLLAR)) {
            expect(TokenType::IDENTIFIER, "Expected type variable name after '$'.");
            return std::make_unique<VariableTypename>(m_previous);
        }

        // Unit/grouped/tuple typename?
        if (consume(TokenType::LEFT_PAREN)) {
            Token paren = m_previous;

            std::vector<std::unique_ptr<const Typename>> elementTypenames;

            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    elementTypenames.push_back(expectTypename("Expected typename after '('."));
                } while (consume(TokenType::COMMA));
            }

            expect(TokenType::RIGHT_PAREN, "Expected ')' after typename elements.");

            // 1-tuples are equivalent to their contained type
            if (elementTypenames.size() == 1) {
                return std::move(elementTypenames[0]);
            } else {
                return std::make_unique<TupleTypename>(std::move(elementTypenames), std::move(paren));
            }
        }

        if (emptyAllowed) {
            return std::make_unique<BasicTypename>("", m_previous);
        }

        throw error(msg);
    }

    std::unique_ptr<BlockExpr> Parser::expectBlock(const std::string &msg) {
        if (!consume(TokenType::EQUAL_GREATER) && !consume(TokenType::LEFT_BRACE)) {
            throw errorAtCurrent(msg);
        }
        return static_unique_ptr_cast<BlockExpr>(parseBlockExpr());
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

    void Parser::synchronise() {
        advance();

        while (!isAtEnd()) {
            switch (m_current.type) {
                case TokenType::ENUM:
                case TokenType::FOR:
                case TokenType::FUNC:
                case TokenType::IMM:
                case TokenType::IMPL:
                case TokenType::MUT:
                case TokenType::STRUCT:
                case TokenType::TRAIT:
                    return;
            }

            advance();
        }
    }

    bool Parser::isAtEnd() {
        return m_current.type == TokenType::END_OF_FILE;
    }
}