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

void Enact::run(const std::string &source) {
    Parser parser{source};
    //if (!compiler.compile()) return InterpretResult::COMPILE_ERROR;
    //std::cout << compiler.currentChunk().disassemble();
    std::vector<Sp<Stmt>> statements = parser.parse();
    if (parser.hadError()) return;

    AstPrinter astPrinter;
    for (const Sp<Stmt>& stmt : statements) {
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
}

void Enact::runPrompt() {
    while (true) {
        std::cout << "enact > ";

        std::string input;
        std::getline(std::cin, input);

        run(input + "\n");
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
    Enact::start(argc, argv);

    return 0;
}