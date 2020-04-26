#ifndef ENACT_TOKEN_H
#define ENACT_TOKEN_H

#include "../common.h"

#include <iostream>

namespace enact {
    enum class TokenType {
        // Single character tokens.
        LEFT_PAREN, RIGHT_PAREN,
        LEFT_SQUARE, RIGHT_SQUARE,
        COLON, COMMA, DOT, MINUS,
        NEWLINE, PLUS, QUESTION,
        SEMICOLON, SEPARATOR, SLASH,
        STAR,

        // 1 or 2 character tokens.
        BANG, BANG_EQUAL,
        EQUAL, EQUAL_EQUAL,
        GREATER, GREATER_EQUAL,
        LESS, LESS_EQUAL,

        // Literals.
        IDENTIFIER, STRING, INTEGER, FLOAT,

        // Reserved words.
        AND, ASSOC, BLOCK, BREAK,
        CLASS, CONTINUE, COPY,
        EACH, ELSE, END, FALSE, FUN,
        FOR, GIVEN, IF, IN, IS, LET, NIL,
        OR, RETURN, STRUCT, THIS, TRAIT,
        TRUE, VAL, VAR, WHEN, WHILE,

        ERROR, ENDFILE, MAX,
    };

    struct Token {
        TokenType type;
        std::string lexeme;
        line_t line;
        col_t col;

        Token(TokenType type, std::string lexeme, line_t line, col_t col) :
                type{type},
                lexeme{lexeme},
                line{line},
                col{col} {}

        static Token synthetic(std::string name) {
            return Token{TokenType::IDENTIFIER, std::move(name), 0, 0};
        }
    };
}

#endif //ENACT_TOKEN_H
