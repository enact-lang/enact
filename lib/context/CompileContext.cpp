#include "CompileContext.h"

#include <sstream>

#include "../AstSerialize.h"

namespace enact {
    CompileContext::CompileContext(Options options) : options{std::move(options)} {
    }

    CompileResult CompileContext::compile(std::string source) {
        m_source = std::move(source);

        // TODO: parse and print AST
    }

    std::string CompileContext::getSourceLine(line_t line) {
        std::istringstream stream{source};
        line_t lineNumber{1};
        std::string lineContents;

        while (std::getline(stream, lineContents) && lineNumber < line) {
            ++lineNumber;
        }

        return lineContents;
    }

    void CompileContext::reportErrorAt(const Token &token, const std::string &msg) {
        std::cerr << "[line " << token.line << "] Error";

        if (token.type == TokenType::ENDFILE) {
            std::cerr << " at end: " << msg << "\n\n";
        } else {
            if (token.type == TokenType::ERROR) {
                std::cerr << ":\n";
            } else {
                std::cerr << " at " << (token.lexeme == "\n" ? "newline" : "'" + token.lexeme + "'") << ":\n";
            }

            std::cerr << "    " << getSourceLine(token.lexeme == "\n" ? token.line - 1 : token.line) << "\n    ";
            for (int i = 1; i <= token.col - token.lexeme.size(); ++i) {
                std::cerr << " ";
            }

            for (int i = 1; i <= token.lexeme.size(); ++i) {
                std::cerr << "^";
            }
            std::cerr << "\n" << msg << "\n\n";
        }
    }
}