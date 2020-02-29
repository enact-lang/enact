#ifndef ENACT_PARSER_H
#define ENACT_PARSER_H

#include <array>
#include <memory>
#include <string>

#include "Chunk.h"
#include "Scanner.h"
#include "Token.h"

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
typedef Expr (Parser::*PrefixFn)();
typedef Expr (Parser::*InfixFn)(Expr);

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
    Expr parsePrecedence(Precedence precedence);

    Expr expression();

    // Prefix parse rules
    Expr grouping();
    Expr variable();
    Expr number();
    Expr literal();
    Expr string();
    Expr array();
    Expr unary();
    Expr reference();

    // Infix parse rules
    Expr call(Expr callee);
    Expr subscript(Expr object);
    Expr binary(Expr left);
    Expr assignment(Expr left);
    Expr field(Expr object);
    Expr ternary(Expr condition);

    std::array<ParseRule, (size_t)TokenType::MAX> m_parseRules = {
            ParseRule{&Parser::grouping,   &Parser::call,          Precedence::CALL}, // LEFT_PAREN
            ParseRule{nullptr,               nullptr,              Precedence::NONE}, // RIGHT_PAREN
            ParseRule{&Parser::array,        &Parser::subscript,   Precedence::CALL}, // LEFT_SQUARE
            ParseRule{nullptr,               nullptr,              Precedence::NONE}, // RIGHT_SQUARE
            ParseRule{&Parser::reference,    nullptr,              Precedence::UNARY}, // AMPERSAND
            ParseRule{nullptr,               nullptr,              Precedence::NONE}, // COLON
            ParseRule{nullptr,               nullptr,              Precedence::NONE}, // COMMA
            ParseRule{nullptr,               &Parser::field,       Precedence::CALL}, // DOT
            ParseRule{&Parser::unary,        &Parser::binary,      Precedence::TERM}, // MINUS
            ParseRule{nullptr,               nullptr,              Precedence::NONE}, // NEWLINE
            ParseRule{nullptr,              &Parser::binary,      Precedence::TERM}, // PLUS
            ParseRule{nullptr,              &Parser::ternary,     Precedence::CONDITIONAL}, // QUESTION
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // SEMICOLON
            ParseRule{nullptr,              &Parser::binary,      Precedence::FACTOR}, // SLASH
            ParseRule{nullptr,              &Parser::binary,      Precedence::FACTOR}, // STAR
            ParseRule{&Parser::unary,       nullptr,              Precedence::UNARY}, // BANG
            ParseRule{nullptr,              &Parser::binary,      Precedence::EQUALITY}, // BANG_EQUAL
            ParseRule{nullptr,              &Parser::assignment,  Precedence::ASSIGNMENT}, // EQUAL
            ParseRule{nullptr,              &Parser::binary,      Precedence::EQUALITY}, // EQUAL_EQUAL
            ParseRule{nullptr,              &Parser::binary,      Precedence::COMPARISON}, // GREATER
            ParseRule{nullptr,              &Parser::binary,      Precedence::COMPARISON}, // GREATER_EQUAL
            ParseRule{nullptr,              &Parser::binary,      Precedence::COMPARISON}, // LESS
            ParseRule{nullptr,              &Parser::binary,      Precedence::COMPARISON}, // LESS_EQUAL
            ParseRule{&Parser::variable,    nullptr,              Precedence::NONE}, // IDENTIFIER
            ParseRule{&Parser::string,      nullptr,              Precedence::NONE}, // STRING
            ParseRule{&Parser::number,      nullptr,              Precedence::NONE}, // INTEGER
            ParseRule{&Parser::number,      nullptr,              Precedence::NONE}, // FLOAT
            ParseRule{nullptr,              &Parser::binary,      Precedence::AND}, // AND
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // ASSOC
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // BLOCK
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // BREAK
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // CLASS
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // CONST
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // CONTINUE
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // EACH
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // ELSE
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // END
            ParseRule{&Parser::literal,     nullptr,              Precedence::NONE}, // FALSE
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // FUN
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // FOR
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // GIVEN
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // IF
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // IN
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // IS
            ParseRule{&Parser::literal,     nullptr,              Precedence::NONE}, // NIL
            ParseRule{nullptr,              &Parser::binary,      Precedence::OR}, // OR
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // RETURN
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // STRUCT
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // THIS
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // TRAIT
            ParseRule{&Parser::literal,     nullptr,              Precedence::NONE}, // TRUE
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // VAR
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // WHEN
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // WHILE
            ParseRule{nullptr,              nullptr,              Precedence::NONE}, // ERROR
            ParseRule{nullptr,              nullptr,              Precedence::NONE} // ENDFILE
    };

    // Declarations
    Stmt declaration();
    Stmt functionDeclaration(bool mustParseBody = true);
    Stmt structDeclaration();
    Stmt traitDeclaration();
    Stmt variableDeclaration(bool isConst);

    // Statements
    Stmt statement();
    Stmt blockStatement();
    Stmt ifStatement();
    Stmt whileStatement();
    Stmt forStatement();
    Stmt eachStatement();
    Stmt givenStatement();
    Stmt returnStatement();
    Stmt breakStatement();
    Stmt continueStatement();
    Stmt expressionStatement();

    std::string consumeTypeName(bool emptyAllowed = false);

    void synchronise();

public:
    explicit Parser(std::string source);
    std::vector<Stmt> parse();
    bool hadError();
};



#endif //ENACT_COMPILER_H
