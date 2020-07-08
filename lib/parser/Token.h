#ifndef ENACT_TOKEN_H
#define ENACT_TOKEN_H

#include "../common.h"

#include <iostream>

namespace enact {
    enum class TokenType {
        // Single character tokens.
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_BRACE, RIGHT_BRACE,
        LEFT_SQUARE, RIGHT_SQUARE,
        AMPERSAND, APOSTROPHE, CARAT, COMMA, HASH,
        MINUS, PIPE, PLUS, QUESTION, SEMICOLON,
        SLASH, STAR, TILDE,

        // 1 or 2 character tokens.
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL, EQUAL_GREATER,
        GREATER, GREATER_EQUAL, GREATER_GREATER,
        LESS, LESS_EQUAL, LESS_LESS,

        // 1, 2, or 3 character tokens.
        DOT, DOT_DOT, DOT_DOT_DOT,

        // Literals.
        IDENTIFIER, INTEGER, FLOAT,
        STRING, INTERPOLATION,

        // Reserved words.
        AND, AS, ASSOC, BREAK, CASE,
        CONTINUE, DEFAULT, ELSE, ENUM, FALSE,
        GC, FUNC, FOR, IF, IMM,
        IMPL, IN, IS, MUT, NOT,
        OR, PUB, RC, RETURN, SO,
        STRUCT, SWITCH, TRAIT, TRUE, WHEN,
        WHILE,

        ERROR, END_OF_FILE, ENUM_MAX,
    };

    struct Token {
        TokenType type;
        std::string lexeme;
        line_t line;
        col_t col;
    };
}

#endif //ENACT_TOKEN_H
