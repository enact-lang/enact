#include <sstream>

#include "Lexer.h"

namespace enact {
    Lexer::Lexer(std::string source) : m_source{std::move(source)} {}

    Token Lexer::scanToken() {
        skipWhitespace();
        m_start = m_current;

        if (isAtEnd()) return makeToken(TokenType::END_OF_FILE);

        char c = advance();

        if (isDigit(c)) return number();
        if (isIdentifierStart(c)) return identifier();

        switch (c) {
            // Single character tokens.
            case '(':
                return makeToken(TokenType::LEFT_PAREN);
            case ')':
                if (m_currentInterpolations > 0) {
                    return interpolationEnd();
                }
                return makeToken(TokenType::RIGHT_PAREN);
            case '{':
                return makeToken(TokenType::LEFT_BRACE);
            case '}':
                return makeToken(TokenType::RIGHT_BRACE);
            case '[':
                return makeToken(TokenType::LEFT_SQUARE);
            case ']':
                return makeToken(TokenType::RIGHT_SQUARE);
            case '&':
                return makeToken(TokenType::AMPERSAND);
            case '\'':
                return makeToken(TokenType::APOSTROPHE);
            case '^':
                return makeToken(TokenType::CARAT);
            case ',':
                return makeToken(TokenType::COMMA);
            case '.':
                return makeToken(TokenType::DOT);
            case '#':
                return makeToken(TokenType::HASH);
            case '-':
                return makeToken(TokenType::MINUS);
            case '|':
                return makeToken(TokenType::PIPE);
            case '+':
                return makeToken(TokenType::PLUS);
            case '?':
                return makeToken(TokenType::QUESTION);
            case ';':
                return makeToken(TokenType::SEMICOLON);
            case '/':
                return makeToken(TokenType::SLASH);
            case '*':
                return makeToken(TokenType::STAR);
            case '~':
                return makeToken(TokenType::TILDE);

            // 1 or 2 character tokens.
            case '!':
                return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            case '=':
                if (match('=')) {
                    return makeToken(TokenType::EQUAL_EQUAL);
                }
                if (match('>')) {
                    return makeToken(TokenType::EQUAL_GREATER);
                }
                return makeToken(TokenType::EQUAL);
            case '>':
                if (match('=')) {
                    return makeToken(TokenType::GREATER_EQUAL);
                }
                if (match('>')) {
                    return makeToken(TokenType::GREATER_GREATER);
                }
                return makeToken(TokenType::GREATER);
            case '<':
                if (match('=')) {
                    return makeToken(TokenType::LESS_EQUAL);
                }
                if (match('<')) {
                    return makeToken(TokenType::LESS_LESS);
                }
                return makeToken(TokenType::LESS);

            case '"':
                return string();
        }

        std::string errorMessage = "Unrecognized character '";
        errorMessage.push_back(c);
        errorMessage.append("'.");
        return errorToken(errorMessage);
    }

    void Lexer::skipWhitespace() {
        while (true) {
            char c = peek();
            switch (c) {
                case '\n':
                    ++m_line;
                    // Fallthrough
                case ' ':
                case '\t':
                case '\r':
                    advance();
                    break;

                case '/':
                    if (peekNext() == '/') {
                        while (peek() != '\n' && !isAtEnd()) advance();
                    } else {
                        return;
                    }

                default:
                    return;
            }
        }
    }

    Token Lexer::number() {
        while (isDigit(peek())) advance();

        TokenType type = TokenType::INTEGER;

        if (peek() == '.') {
            type = TokenType::FLOAT;
            advance();
            while (isDigit(peek())) advance();
        }

        return makeToken(type);
    }

    Token Lexer::identifier() {
        while (isIdentifier(peek())) advance();
        return makeToken(getIdentifierType(m_source.substr(m_start, m_current - m_start)));
    }

    Token Lexer::string() {
        std::string value;
        bool inEscapeSequence = false;
        while (!isAtEnd()) {
            const char c = peek();

            if (inEscapeSequence) {
                switch (c) {
                    case 'n':
                        value.push_back('\n');
                        break;
                    case 'r':
                        value.push_back('\r');
                        break;
                    case 't':
                        value.push_back('\t');
                        break;
                    case '\\':
                        value.push_back('\\');
                        break;
                    case '"':
                        value.push_back('"');
                        break;
                    case '(':
                        // Eat the '('
                        advance();
                        return interpolationStart(std::move(value));
                    default:
                        advance();
                        return errorToken("Unrecognised escape sequence.");
                }

                inEscapeSequence = false;
            } else {
                if (c == '"') break;
                if (c == '\\') {
                    inEscapeSequence = true;
                    advance();
                    continue;
                }

                value.push_back(c);
            }

            advance();
        }

        if (isAtEnd()) {
            return errorToken("Unterminated string.");
        }

        // Eat the close quote.
        advance();

        return Token{TokenType::STRING, value, m_line, m_col};
    }

    Token Lexer::interpolationStart(std::string value) {
        ++m_currentInterpolations;
        return Token{TokenType::INTERPOLATION, std::move(value), m_line, m_col};
    }

    Token Lexer::interpolationEnd() {
        --m_currentInterpolations;
        return string();
    }

    Token Lexer::makeToken(TokenType type) {
        std::string lexeme{m_source.substr(m_start, m_current - m_start)};
        m_last = Token{type, lexeme, m_line, m_col};
        return m_last;
    }

    Token Lexer::errorToken(const std::string &what) {
        return Token{TokenType::ERROR, what, m_line, m_col};
    }

    TokenType Lexer::getIdentifierType(const std::string& candidate) {
        if (m_keywords.count(candidate) > 0) {
            return m_keywords.at(candidate);
        }

        return TokenType::IDENTIFIER;
    }

    bool Lexer::isAtEnd() {
        return m_current >= m_source.length();
    }

    char Lexer::advance() {
        ++m_col;
        return m_source[m_current++];
    }

    char Lexer::peek() {
        return m_source[m_current];
    }

    char Lexer::peekNext() {
        return m_source[m_current + 1];
    }

    char Lexer::previous() {
        return m_source[m_current - 1];
    }

    bool Lexer::match(char expected) {
        if (peek() == expected) {
            advance();
            return true;
        }
        return false;
    }

    bool Lexer::isDigit(char c) {
        return c >= '0' && c <= '9';
    }

    bool Lexer::isIdentifierStart(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_';
    }

    bool Lexer::isIdentifier(char c) {
        return isIdentifierStart(c) || isDigit(c);
    }
}