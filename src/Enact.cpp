#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "h/Chunk.h"
#include "h/Parser.h"
#include "h/Enact.h"
#include "h/VM.h"
#include "h/AstPrinter.h"
#include "h/Analyser.h"

#include "h/Value.h"

std::string Enact::m_source = "";
Analyser Enact::m_analyser{};

void Enact::run(const std::string &source) {
    m_source = source;

    Parser parser{m_source};
    //if (!compiler.compile()) return InterpretResult::COMPILE_ERROR;
    //std::cout << compiler.currentChunk().disassemble();
    std::vector<Stmt> statements = parser.parse();

    m_analyser.analyse(statements);

    if (parser.hadError() || m_analyser.hadError()) return;

    AstPrinter astPrinter;
    for (const Stmt& stmt : statements) {
        astPrinter.print(stmt);
        std::cout << "\n";
    }

    //return m_vm.run(compiler.currentChunk());
}

void Enact::runFile(const std::string &path) {
    // Get the file contents.
    std::ifstream file{path};

    // Check that the file opened successfully
    if (!file.is_open()) {
        std::cerr << "Error: Unable to read file '" + path + "'.";
        std::exit((int) ExitCode::FILE_ERROR);
    }

    std::stringstream fileContents;
    std::string currentLine;

    while (std::getline(file, currentLine)) {
        fileContents << currentLine << "\n";
    }

    run(fileContents.str());
}

void Enact::runPrompt() {
    while (true) {
        std::cout << "enact > ";

        std::string input;
        std::getline(std::cin, input);

        run(input + "\n");
    }
}

std::string Enact::getSourceLine(const line_t line) {
    std::istringstream source{m_source};
    line_t lineNumber{1};
    std::string lineContents;

    while (std::getline(source, lineContents) && lineNumber < line) {
        ++lineNumber;
    }

    return lineContents;
}

void Enact::reportErrorAt(const Token &token, const std::string &message) {
    std::cerr << "[line " << token.line << "] Error";

    if (token.type == TokenType::ENDFILE) {
        std::cerr << " at end: " << message << "\n\n";
    } else {
        if (token.type == TokenType::ERROR) {
            std::cerr << ":\n";
        } else {
            std::cerr << " at " << (token.lexeme == "\n" ? "newline" : "'" + token.lexeme + "'") << ":\n";
        }

        std::cerr << "    " << getSourceLine(token.lexeme == "\n" ? token.line - 1 : token.line) << "\n";
        for (int i = 1; i < token.col; ++i) {
            std::cerr << " ";
        }
        std::cerr << "    ^\n";
        std::cerr << message << "\n\n";
    }
}

void Enact::start(int argc, char *argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: enact [file]\n";
        std::exit((int)ExitCode::INVALID_ARGUMENTS);
    } else if (argc == 2) {
        // Run the provided file.
        runFile(argv[1]);
    } else {
        // No arguments, initialize REPL.
        runPrompt();
    }
}

int main(int argc, char *argv[]) {
    Chunk chunk;
    chunk.writeConstant(Value{1.2}, 1);
    chunk.writeConstant(Value{21}, 1);
    chunk.write(OpCode::ADD, 1);
    chunk.write(OpCode::RETURN, 1);

    std::cout << chunk.disassemble() << "\n";

    VM vm;
    vm.run(chunk);

    Enact::start(argc, argv);
    return 0;
}