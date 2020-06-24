#ifndef ENACT_PARSER_H
#define ENACT_PARSER_H

#include <array>
#include <memory>
#include <string>

#include "../ast/Stmt.h"
#include "../bytecode/Chunk.h"

#include "Lexer.h"
#include "Token.h"
#include "Typename.h"

namespace enact {
    class CompileContext;

    enum class Precedence {
        NONE,
        ASSIGNMENT,    // =
        LOGICAL_OR,    // or
        LOGICAL_AND,   // and
        EQUALITY,      // == !=
        COMPARISON,    // < > <= >=
        CAST,          // as is
        RANGE,         // .. ...
        BITWISE_OR,    // |
        BITWISE_XOR,   // ^
        BITWISE_AND,   // &
        ADD,           // + -
        MULTIPLY,      // * / %
        BITWISE_SHIFT, // << >>
        UNARY,         // - not ~ & *
        CALL,          // ()
        PRIMARY,
    };

    class Parser;

    // For our Pratt parse table
    typedef std::unique_ptr<Expr> (Parser::*PrefixFn)();
    typedef std::unique_ptr<Expr> (Parser::*InfixFn)(std::unique_ptr<Expr>);

    struct ParseRule {
        PrefixFn prefix;
        InfixFn infix;
        Precedence precedence;
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError() : std::runtime_error{"Uncaught ParseError: Internal"} {}
    };

    class Parser {
    public:
        explicit Parser(CompileContext &context);
        ~Parser() = default;

        std::vector<std::unique_ptr<Stmt>> parse();

        bool hadError() const;

    private:
        std::unique_ptr<Expr> parseAtPrecedence(Precedence precedence);

        std::unique_ptr<Expr> parseExpression();

        // Prefix parse rules
        std::unique_ptr<Expr> parseGroupingExpr();
        std::unique_ptr<Expr> parseLiteralExpr();
        std::unique_ptr<Expr> parseUnaryExpr();

        // Infix parse rules
        std::unique_ptr<Expr> parseCallExpr(std::unique_ptr<Expr> callee);
        std::unique_ptr<Expr> parseFieldExpr(std::unique_ptr<Expr> object);
        std::unique_ptr<Expr> parseBinaryExpr(std::unique_ptr<Expr> left);
        std::unique_ptr<Expr> parseAssignmentExpr(std::unique_ptr<Expr> target);

        // Complex prefix expressions
        std::unique_ptr<Expr> parseBlockExpr();
        std::unique_ptr<Expr> parseIfExpr();
        std::unique_ptr<Expr> parseWhileExpr();
        std::unique_ptr<Expr> parseForExpr();
        std::unique_ptr<Expr> parseSwitchExpr();

        // Declaration statements
        std::unique_ptr<Stmt> parseDeclaration();

        std::unique_ptr<Stmt> parseFunctionStmt(bool mustParseBody = true, bool isMut = false);
        std::unique_ptr<Stmt> parseStructStmt();
        std::unique_ptr<Stmt> parseTraitStmt();
        std::unique_ptr<Stmt> parseVariableStmt(bool isConst, bool mustExpectSeparator = true);

        // Statements
        std::unique_ptr<Stmt> parseStatement();

        std::unique_ptr<Stmt> parseReturnStmt();
        std::unique_ptr<Stmt> parseBreakStmt();
        std::unique_ptr<Stmt> parseContinueStmt();
        std::unique_ptr<Stmt> parseExpressionStmt();

        void advance();
        bool check(TokenType expected);
        bool consume(TokenType expected);
        void expect(TokenType type, const std::string &message);
        bool isAtEnd();

        std::unique_ptr<const Typename> expectTypename(bool emptyAllowed = false);
        std::unique_ptr<const Typename> expectFunctionTypename();

        ParseError errorAt(const Token &token, const std::string &message);
        ParseError errorAtCurrent(const std::string &message);
        ParseError error(const std::string &message);
        void synchronise();

        const ParseRule& getParseRule(TokenType type);

        CompileContext &m_context;

        Lexer m_scanner{""};

        Token m_previous{};
        Token m_current{};

        bool m_hadError = false;

        std::array<ParseRule, (size_t) TokenType::ENUM_MAX> m_parseRules = {
                ParseRule{&Parser::grouping, &Parser::call, Precedence::CALL}, // LEFT_PAREN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // RIGHT_PAREN
                ParseRule{&Parser::array, &Parser::subscript, Precedence::CALL}, // LEFT_SQUARE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // RIGHT_SQUARE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // COLON
                ParseRule{nullptr, nullptr, Precedence::NONE}, // COMMA
                ParseRule{nullptr, &Parser::field, Precedence::CALL}, // DOT
                ParseRule{&Parser::unary, &Parser::binary, Precedence::TERM}, // MINUS
                ParseRule{nullptr, nullptr, Precedence::NONE}, // NEWLINE
                ParseRule{nullptr, &Parser::binary, Precedence::TERM}, // PLUS
                ParseRule{nullptr, &Parser::ternary, Precedence::CONDITIONAL}, // QUESTION
                ParseRule{nullptr, nullptr, Precedence::NONE}, // SEMICOLON
                ParseRule{nullptr, nullptr, Precedence::NONE}, // SEPARATOR
                ParseRule{nullptr, &Parser::binary, Precedence::FACTOR}, // SLASH
                ParseRule{nullptr, &Parser::binary, Precedence::FACTOR}, // STAR
                ParseRule{&Parser::unary, nullptr, Precedence::UNARY}, // BANG
                ParseRule{nullptr, &Parser::binary, Precedence::EQUALITY}, // BANG_EQUAL
                ParseRule{nullptr, &Parser::assignment, Precedence::ASSIGNMENT}, // EQUAL
                ParseRule{nullptr, &Parser::binary, Precedence::EQUALITY}, // EQUAL_EQUAL
                ParseRule{nullptr, &Parser::binary, Precedence::COMPARISON}, // GREATER
                ParseRule{nullptr, &Parser::binary, Precedence::COMPARISON}, // GREATER_EQUAL
                ParseRule{nullptr, &Parser::binary, Precedence::COMPARISON}, // LESS
                ParseRule{nullptr, &Parser::binary, Precedence::COMPARISON}, // LESS_EQUAL
                ParseRule{&Parser::variable, nullptr, Precedence::NONE}, // IDENTIFIER
                ParseRule{&Parser::string, nullptr, Precedence::NONE}, // STRING
                ParseRule{&Parser::number, nullptr, Precedence::NONE}, // INTEGER
                ParseRule{&Parser::number, nullptr, Precedence::NONE}, // FLOAT
                ParseRule{nullptr, &Parser::binary, Precedence::AND}, // AND
                ParseRule{nullptr, nullptr, Precedence::NONE}, // ASSOC
                ParseRule{nullptr, nullptr, Precedence::NONE}, // BLOCK
                ParseRule{nullptr, nullptr, Precedence::NONE}, // BREAK
                ParseRule{nullptr, nullptr, Precedence::NONE}, // CLASS
                ParseRule{nullptr, nullptr, Precedence::NONE}, // CONST
                ParseRule{nullptr, nullptr, Precedence::NONE}, // CONTINUE
                ParseRule{&Parser::unary, nullptr, Precedence::UNARY}, // COPY
                ParseRule{nullptr, nullptr, Precedence::NONE}, // EACH
                ParseRule{nullptr, nullptr, Precedence::NONE}, // ELSE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // END
                ParseRule{&Parser::literal, nullptr, Precedence::NONE}, // FALSE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // FUN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // FOR
                ParseRule{nullptr, nullptr, Precedence::NONE}, // GIVEN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // IF
                ParseRule{nullptr, nullptr, Precedence::NONE}, // IN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // IS
                ParseRule{nullptr, nullptr, Precedence::NONE}, // MUT
                ParseRule{&Parser::literal, nullptr, Precedence::NONE}, // NIL
                ParseRule{nullptr, &Parser::binary, Precedence::OR}, // OR
                ParseRule{nullptr, nullptr, Precedence::NONE}, // RETURN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // STRUCT
                ParseRule{nullptr, nullptr, Precedence::NONE}, // THIS
                ParseRule{nullptr, nullptr, Precedence::NONE}, // TRAIT
                ParseRule{&Parser::literal, nullptr, Precedence::NONE}, // TRUE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // VAR
                ParseRule{nullptr, nullptr, Precedence::NONE}, // WHEN
                ParseRule{nullptr, nullptr, Precedence::NONE}, // WHILE
                ParseRule{nullptr, nullptr, Precedence::NONE}, // ERROR
                ParseRule{nullptr, nullptr, Precedence::NONE} // ENDFILE
        };
    };
}

#endif //ENACT_COMPILER_H
