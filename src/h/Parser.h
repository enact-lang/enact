#ifndef ENACT_PARSER_H
#define ENACT_PARSER_H

#include <array>
#include <memory>
#include <string>

#include "Chunk.h"
#include "Scanner.h"
#include "Token.h"
#include "Typename.h"

#include "../ast/Stmt.h"

enum class Precedence {
    NONE,
    ASSIGNMENT,  // =
    CONDITIONAL, // ?:
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // - !
    CALL,        // () []
    PRIMARY,
};

class Parser;
typedef std::unique_ptr<Expr> (Parser::*PrefixFn)();
typedef std::unique_ptr<Expr> (Parser::*InfixFn)(std::unique_ptr<Expr>);

struct ParseRule {
    PrefixFn prefix;
    InfixFn infix;
    Precedence precedence;
};

class Parser {
private:
    class ParseError : public std::runtime_error {
    public:
        ParseError() : std::runtime_error{"Uncaught ParseError: Internal"} {}
    };

    std::string m_source;
    Scanner m_scanner;

    bool m_hadError = false;
    bool m_panicMode = false;

    Token m_previous;
    Token m_current;

    void advance();
    void undoAdvance();
    void ignoreNewline();
    bool check(TokenType expected);
    bool consume(TokenType expected);
    bool consumeSeparator();
    void expect(TokenType type, const std::string &message);
    void expectSeparator(const std::string &message);
    bool isAtEnd();
    
    ParseError errorAt(const Token &token, const std::string &message);
    ParseError errorAtCurrent(const std::string &message);
    ParseError error(const std::string &message);

    const ParseRule& getParseRule(TokenType type);
    std::unique_ptr<Expr> parsePrecedence(Precedence precedence);

    std::unique_ptr<Expr> expression();

    // Prefix parse rules
    std::unique_ptr<Expr> grouping();
    std::unique_ptr<Expr> variable();
    std::unique_ptr<Expr> number();
    std::unique_ptr<Expr> literal();
    std::unique_ptr<Expr> string();
    std::unique_ptr<Expr> array();
    std::unique_ptr<Expr> unary();

    // Infix parse rules
    std::unique_ptr<Expr> call(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> subscript(std::unique_ptr<Expr> object);
    std::unique_ptr<Expr> binary(std::unique_ptr<Expr> left);
    std::unique_ptr<Expr> assignment(std::unique_ptr<Expr> target);
    std::unique_ptr<Expr> field(std::unique_ptr<Expr> object);
    std::unique_ptr<Expr> ternary(std::unique_ptr<Expr> condition);

    std::array<ParseRule, (size_t)TokenType::MAX> m_parseRules = {
            ParseRule{&Parser::grouping,   &Parser::call,    Precedence::CALL}, // LEFT_PAREN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // RIGHT_PAREN
            ParseRule{&Parser::array,      &Parser::subscript,            Precedence::CALL}, // LEFT_SQUARE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // RIGHT_SQUARE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // COLON
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // COMMA
            ParseRule{nullptr,               &Parser::field,            Precedence::CALL}, // DOT
            ParseRule{&Parser::unary,      &Parser::binary,  Precedence::TERM}, // MINUS
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // NEWLINE
            ParseRule{nullptr,               &Parser::binary,  Precedence::TERM}, // PLUS
            ParseRule{nullptr,               &Parser::ternary, Precedence::CONDITIONAL}, // QUESTION
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // SEMICOLON
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // SEPARATOR
            ParseRule{nullptr,               &Parser::binary,  Precedence::FACTOR}, // SLASH
            ParseRule{nullptr,               &Parser::binary,  Precedence::FACTOR}, // STAR
            ParseRule{&Parser::unary,      nullptr,            Precedence::UNARY}, // BANG
            ParseRule{nullptr,               &Parser::binary,  Precedence::EQUALITY}, // BANG_EQUAL
            ParseRule{nullptr,               &Parser::assignment,  Precedence::ASSIGNMENT}, // EQUAL
            ParseRule{nullptr,               &Parser::binary,  Precedence::EQUALITY}, // EQUAL_EQUAL
            ParseRule{nullptr,               &Parser::binary,  Precedence::COMPARISON}, // GREATER
            ParseRule{nullptr,               &Parser::binary,  Precedence::COMPARISON}, // GREATER_EQUAL
            ParseRule{nullptr,               &Parser::binary,  Precedence::COMPARISON}, // LESS
            ParseRule{nullptr,               &Parser::binary,  Precedence::COMPARISON}, // LESS_EQUAL
            ParseRule{&Parser::variable,   nullptr,            Precedence::NONE}, // IDENTIFIER
            ParseRule{&Parser::string,     nullptr,            Precedence::NONE}, // STRING
            ParseRule{&Parser::number,     nullptr,            Precedence::NONE}, // INTEGER
            ParseRule{&Parser::number,     nullptr,            Precedence::NONE}, // FLOAT
            ParseRule{nullptr,               &Parser::binary,            Precedence::AND}, // AND
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // ASSOC
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // BLOCK
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // BREAK
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // CLASS
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // CONST
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // CONTINUE
            ParseRule{&Parser::unary,               nullptr,            Precedence::UNARY}, // COPY
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // EACH
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // ELSE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // END
            ParseRule{&Parser::literal,    nullptr,            Precedence::NONE}, // FALSE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // FUN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // FOR
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // GIVEN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // IF
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // IN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // IS
            ParseRule{&Parser::literal,    nullptr,            Precedence::NONE}, // NIL
            ParseRule{nullptr,               &Parser::binary,            Precedence::OR}, // OR
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // RETURN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // STRUCT
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // THIS
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // TRAIT
            ParseRule{&Parser::literal,    nullptr,            Precedence::NONE}, // TRUE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // VAR
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // WHEN
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // WHILE
            ParseRule{nullptr,               nullptr,            Precedence::NONE}, // ERROR
            ParseRule{nullptr,               nullptr,            Precedence::NONE} // ENDFILE
    };

    // Declarations
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> functionDeclaration(bool mustParseBody = true);
    std::unique_ptr<Stmt> structDeclaration();
    std::unique_ptr<Stmt> traitDeclaration();
    std::unique_ptr<Stmt> variableDeclaration(bool isConst);

    // Statements
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> blockStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> forStatement();
    std::unique_ptr<Stmt> eachStatement();
    std::unique_ptr<Stmt> givenStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> breakStatement();
    std::unique_ptr<Stmt> continueStatement();
    std::unique_ptr<Stmt> expressionStatement();

    std::unique_ptr<const Typename> expectTypename(bool emptyAllowed = false);
    std::unique_ptr<const Typename> expectFunctionTypename();

    void synchronise();

public:
    explicit Parser(std::string source);
    std::vector<std::unique_ptr<Stmt>> parse();
    bool hadError();
};



#endif //ENACT_COMPILER_H
