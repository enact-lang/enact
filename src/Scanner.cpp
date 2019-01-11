#include <sstream>

#include "h/Scanner.h"

Scanner::Scanner(std::string source) : m_source{std::move(source)} {}

Token Scanner::scanToken() {
    skipWhitespace();
    m_start = m_current;

    if (isAtEnd()) return makeToken(TokenType::ENDFILE);

    char c = advance();

    if (isDigit(c)) return number();
    if (isIdentifierStart(c)) return identifier();

    switch (c) {
        // Single character tokens.
        case '(': ++m_openParen; return makeToken(TokenType::LEFT_PAREN);
        case ')': --m_openParen; return makeToken(TokenType::RIGHT_PAREN);
        case '[': ++m_openSquare; return makeToken(TokenType::LEFT_SQUARE);
        case ']': --m_openSquare; return makeToken(TokenType::RIGHT_SQUARE);
        case ':': return makeToken(TokenType::COLON);
        case ',': return makeToken(TokenType::COMMA);
        case '.': return makeToken(TokenType::DOT);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case '?': return makeToken(TokenType::QUESTION);
        case ';': return makeToken(TokenType::SEMICOLON);
        case '/': return makeToken(TokenType::SLASH);
        case '*': return makeToken(TokenType::STAR);

            // 1 or 2 character tokens.
        case '!':
            return makeToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        case '=':
            return makeToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '>':
            return makeToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '<':
            return makeToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);

        case '"':
            return string();
    }

    std::string errorMessage = "Unrecognized character '";
    errorMessage.append(std::string{c});
    errorMessage.append("'.");
    return errorToken(errorMessage);
}

void Scanner::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;

            case '\n':
                ++m_line;
                advance();
                break;

            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                    match('\n');
                } else {
                    return;
                }

            default:
                return;
        }
    }
}

bool Scanner::consumeSeparator() {
    if (previous() == '\n' || previous() == ';') {
        advance();
        return true;
    }
    return false;
}

Token Scanner::number() {
    while (isDigit(peek())) advance();

    if (peek() == '.') {
        advance();
        while (isDigit(peek())) advance();
    }

    return makeToken(TokenType::NUMBER);
}

Token Scanner::identifier() {
    while (isIdentifier(peek())) advance();
    return makeToken(identifierType(m_source.substr(m_start, m_current - m_start)));
}

Token Scanner::string() {
    while (peek() != '"' && !isAtEnd()) advance();

    if (isAtEnd()) {
        return errorToken("Unterminated string.");
    }

    // Eat the close quote.
    advance();

    return makeToken(TokenType::STRING);
}

Token Scanner::makeToken(TokenType type) {
    std::string lexeme{m_source.substr(m_start, m_current - m_start)};
    m_last = Token{type, lexeme, m_line, m_col};
    return m_last;
}

Token Scanner::errorToken(const std::string &what) {
    return Token{TokenType::ERROR, what, m_line, m_col};
}

TokenType Scanner::identifierType(std::string candidate) {
    if (candidate == "and")     return TokenType::AND;
    if (candidate == "bool")    return TokenType::BOOL;
    if (candidate == "class")   return TokenType::CLASS;
    if (candidate == "const")   return TokenType::CONST;
    if (candidate == "else")    return TokenType::ELSE;
    if (candidate == "end")     return TokenType::END;
    if (candidate == "false")   return TokenType::FALSE;
    if (candidate == "consumeSeparator")     return TokenType::FUN;
    if (candidate == "for")     return TokenType::FOR;
    if (candidate == "if")      return TokenType::IF;
    if (candidate == "nil")     return TokenType::NIL;
    if (candidate == "or")      return TokenType::OR;
    if (candidate == "return")  return TokenType::RETURN;
    if (candidate == "super")   return TokenType::SUPER;
    if (candidate == "this")    return TokenType::THIS;
    if (candidate == "true")    return TokenType::TRUE;
    if (candidate == "var")     return TokenType::VAR;
    if (candidate == "while")   return TokenType::WHILE;

    return TokenType::IDENTIFIER;
}

std::string Scanner::getSourceLine(line_t line) {
    std::istringstream source{m_source};
    line_t lineNumber{1};
    std::string lineContents;

    while (std::getline(source, lineContents) && lineNumber < line) {
        ++lineNumber;
    }

    return lineContents;
}

bool Scanner::isAtEnd() {
    return m_current >= m_source.length();
}

char Scanner::advance() {
    ++m_col;
    return m_source[m_current++];
}

char Scanner::peek() {
    return m_source[m_current];
}

char Scanner::peekNext() {
    return m_source[m_current + 1];
}

char Scanner::previous() {
    return m_source[m_current - 1];
}

bool Scanner::match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

bool Scanner::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Scanner::isIdentifierStart(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Scanner::isIdentifier(char c) {
    return isIdentifierStart(c) || isDigit(c);
}
