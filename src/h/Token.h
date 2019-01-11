#ifndef ENACT_TOKEN_H
#define ENACT_TOKEN_H

#include "common.h"

#include <iostream>

enum class TokenType {
    // Single character tokens.
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_SQUARE, RIGHT_SQUARE,
    COLON, COMMA, DOT, MINUS, PLUS,
    QUESTION, SEMICOLON, SLASH, STAR,

    // 1 or 2 character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Reserved words.
    AND, BOOL, CLASS, CONST, ELSE, END, FALSE, FUN, FOR,
    IF, NIL, OR, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    ERROR, ENDFILE, MAX_TOKEN_TYPES,
};

struct Token {
    TokenType type;
    std::string lexeme;
    line_t line;
    col_t col;
};


#endif //ENACT_TOKEN_H
