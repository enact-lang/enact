#ifndef ENACT_TOKEN_H
#define ENACT_TOKEN_H

#include "common.h"

#include <iostream>

enum class TokenType {
    // Single character tokens.
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_SQUARE, RIGHT_SQUARE,
    COLON, COMMA, DOT, MINUS,
    NEWLINE, PLUS, QUESTION,
    SEMICOLON, SLASH, STAR,

    // 1 or 2 character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, INTEGER, FLOAT,

    // Reserved words.
    AND, ASSOC, BLOCK, BREAK,
    CLASS, CONST, CONTINUE, EACH,
    ELSE, END, FALSE, FUN, FOR,
    GIVEN, IF, IN, IS, NIL, OR,
    RETURN, STRUCT, THIS, TRAIT,
    TRUE, VAR, WHEN, WHILE,

    ERROR, ENDFILE, MAX,
};

struct Token {
    TokenType type;
    std::string lexeme;
    line_t line;
    col_t col;
};

#endif //ENACT_TOKEN_H
