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
        CALL,          // () .
        PRIMARY,
    };

    class Parser;

    // For our Pratt parse table
    typedef std::unique_ptr<Expr> (Parser::*PrefixFn)();
    typedef std::unique_ptr<Expr> (Parser::*InfixFn)(std::unique_ptr<Expr>);

    struct ParseRule {
        PrefixFn prefix;
        InfixFn infix;
        Precedence parsePrecedence;
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
        std::unique_ptr<Stmt> parseStmt();

        std::unique_ptr<Stmt> parseFunctionStmt(bool mustParseBody = true);
        std::unique_ptr<Stmt> parseStructStmt();
        std::unique_ptr<Stmt> parseEnumStmt();
        std::unique_ptr<Stmt> parseTraitStmt();
        std::unique_ptr<Stmt> parseImplStmt();
        std::unique_ptr<Stmt> parseVariableStmt();
        std::unique_ptr<Stmt> parseReturnStmt();
        std::unique_ptr<Stmt> parseBreakStmt();
        std::unique_ptr<Stmt> parseContinueStmt();
        std::unique_ptr<Stmt> parseExpressionStmt();

        std::unique_ptr<Expr> parseExpr();

        std::unique_ptr<Expr> parseBlockExpr();
        std::unique_ptr<Expr> parseIfExpr();
        std::unique_ptr<Expr> parseWhileExpr();
        std::unique_ptr<Expr> parseForExpr();
        std::unique_ptr<Expr> parseSwitchExpr();

        std::unique_ptr<Expr> parsePrecAssignment();
        std::unique_ptr<Expr> parsePrecLogicalOr();
        std::unique_ptr<Expr> parsePrecLogicalAnd();
        std::unique_ptr<Expr> parsePrecEquality();
        std::unique_ptr<Expr> parsePrecComparison();
        std::unique_ptr<Expr> parsePrecCast();
        std::unique_ptr<Expr> parsePrecRange();
        std::unique_ptr<Expr> parsePrecBitwiseOr();
        std::unique_ptr<Expr> parsePrecBitwiseXor();
        std::unique_ptr<Expr> parsePrecBitwiseAnd();
        std::unique_ptr<Expr> parsePrecAdd();
        std::unique_ptr<Expr> parsePrecMultiply();
        std::unique_ptr<Expr> parsePrecBitwiseShift();
        std::unique_ptr<Expr> parsePrecUnary();
        std::unique_ptr<Expr> parsePrecCall();
        std::unique_ptr<Expr> parsePrecPrimary();

        void advance();
        bool check(TokenType expected);
        bool consume(TokenType expected);
        Token expect(TokenType type, const std::string &message);
        bool isAtEnd();

        std::unique_ptr<const Typename> expectTypename(const std::string& msg, bool emptyAllowed = false);
        std::unique_ptr<const Typename> expectFunctionTypename(const std::string& msg);

        ParseError errorAt(const Token &token, const std::string &message);
        ParseError errorAtCurrent(const std::string &message);
        ParseError error(const std::string &message);
        void synchronise();

        CompileContext &m_context;

        Lexer m_scanner{""};

        Token m_previous{};
        Token m_current{};

        bool m_hadError = false;
    };
}

#endif //ENACT_COMPILER_H
