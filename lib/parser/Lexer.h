#ifndef ENACT_LEXER_H
#define ENACT_LEXER_H

#include <string>

#include "Token.h"

namespace enact {
    class Lexer {
    public:
        explicit Lexer(std::string source);
        ~Lexer() = default;

        Token scanToken();

    private:
        Token number();
        Token identifier();
        Token string();

        Token interpolationStart(std::string value);
        Token interpolationEnd();

        TokenType getIdentifierType(const std::string& candidate);

        Token makeToken(TokenType type);
        Token errorToken(const std::string &what);

        void skipWhitespace();
        bool isAtEnd();

        char advance();
        char peek();
        char peekNext();
        char previous();

        bool match(char expected);

        bool isDigit(char c);
        bool isIdentifierStart(char c);
        bool isIdentifier(char c);

        std::string m_source;
        size_t m_start, m_current = 0;

        line_t m_line = 1;
        col_t m_col = 0;

        Token m_last;

        int m_currentInterpolations = 0;

        std::unordered_map<std::string, TokenType> m_keywords{
                {"and", TokenType::AND},
                {"as", TokenType::AS},
                {"assoc", TokenType::ASSOC},
                {"break", TokenType::BREAK},
                {"case", TokenType::CASE},
                {"continue", TokenType::CONTINUE},
                {"default", TokenType::DEFAULT},
                {"else", TokenType::ELSE},
                {"enum", TokenType::ENUM},
                {"false", TokenType::FALSE},
                {"func", TokenType::FUNC},
                {"for", TokenType::FOR},
                {"if", TokenType::IF},
                {"imm", TokenType::IMM},
                {"impl", TokenType::IMPL},
                {"in", TokenType::IN},
                {"is", TokenType::IS},
                {"mut", TokenType::MUT},
                {"not", TokenType::NOT},
                {"or", TokenType::OR},
                {"pub", TokenType::PUB},
                {"return", TokenType::RETURN},
                {"struct", TokenType::STRUCT},
                {"switch", TokenType::SWITCH},
                {"trait", TokenType::TRAIT},
                {"true", TokenType::TRUE},
                {"when", TokenType::WHEN},
                {"while", TokenType::WHILE},
        };
    };
}


#endif //ENACT_LEXER_H
