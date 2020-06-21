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
        AMPERSAND, APOSTROPHE, COMMA, DOT, HASH,
        MINUS, PLUS, QUESTION, SEMICOLON, SLASH, STAR,

        // 1 or 2 character tokens.
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,

        // Literals.
        IDENTIFIER, INTEGER, FLOAT,
        STRING, INTERPOLATION,

        // Reserved words.
        AND, AS, ASSOC, BREAK, CONTINUE,
        ELSE, ENUM, FALSE, FUNC, FOR,
        IF, IMM, IMPL, IN, IS,
        MUT, NOT, OR, PUB, RETURN,
        STRUCT, TRAIT, TRUE, WHILE,

        ERROR, END_OF_FILE, MAX,
    };

    struct Token {
        TokenType type;
        std::string lexeme;
        line_t line;
        col_t col;
    };
}

#endif //ENACT_TOKEN_H
