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
#include "h/Object.h"
#include "h/Compiler.h"
#include "h/GC.h"

std::string Enact::m_source{};

Flags Enact::m_flags{};
std::string Enact::m_filename{};
std::vector<std::string> Enact::m_programArgs{};

void Enact::start(int argc, char *argv[]) {
    parseArgv(argc, argv);

    if (m_filename.empty()) {
        runPrompt();
    } else {
        runFile(m_filename);
    }
}

InterpretResult Enact::run(const std::string& source) {
    m_source = source;
    FunctionObject* script;

    { // Free up memory for the VM
        Parser parser{m_source};
        std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

        Analyser analyser{};
        statements = analyser.analyse(std::move(statements));

        if (parser.hadError()) return InterpretResult::PARSE_ERROR;
        if (analyser.hadError()) return InterpretResult::ANALYSIS_ERROR;

        if (getFlags().flagEnabled(Flag::DEBUG_PRINT_AST)) {
            AstPrinter astPrinter;
            for (const auto& stmt : statements) {
                astPrinter.print(*stmt);
                std::cout << "\n";
            }
        }

        Compiler compiler{};
        compiler.init(FunctionKind::SCRIPT, std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{}), "");
        compiler.compile(std::move(statements));
        script = compiler.end();
        if (compiler.hadError()) return InterpretResult::COMPILE_ERROR;
    }

    if (getFlags().flagEnabled(Flag::DEBUG_DISASSEMBLE_CHUNK)) {
        std::cout << script->getChunk().disassemble();
    }

    VM vm = VM{};
    InterpretResult result = vm.run(script);

    GC::freeObjects();
    return result;
}

void Enact::runFile(const std::string &path) {
    // Get the file contents.
    std::ifstream file{path};

    // Check that the file opened successfully
    if (!file.is_open()) {
        std::cerr << "[enact] Error: Unable to read file '" + path + "'.";
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

        std::cerr << "    " << getSourceLine(token.lexeme == "\n" ? token.line - 1 : token.line) << "\n    ";
        for (int i = 1; i <= token.col - token.lexeme.size(); ++i) {
            std::cerr << " ";
        }

        for (int i = 1; i <= token.lexeme.size(); ++i) {
            std::cerr << "^";
        }
        std::cerr << "\n" << message << "\n\n";
    }
}

void Enact::parseArgv(int argc, char **argv) {
    if (argc == 0) return;
    std::vector<std::string> args{argv + 1, argv + argc};

    size_t current = 0;
    for (; current < args.size(); ++current) {
        std::string arg = args[current];

        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            m_flags.parseString(arg);
        } else if (arg.size() >= 1 && arg[0] == '-') {
            for (char c : arg.substr(1)) {
                m_flags.parseString(std::string{"-"} + c);
            }
        } else {
            break;
        }
    }

    if (m_flags.hadError()) {
        exit((int)ExitCode::INVALID_ARGUMENTS);
    }

    if (current >= args.size()) {
        // There are no more arguments for us to parse.
        return;
    }

    m_filename = args[current++];

    while (current < args.size()) {
        m_programArgs.push_back(args[current++]);
    }
}

const Flags& Enact::getFlags() {
    return m_flags;
}

const std::vector<std::string>& Enact::getProgramArgs() {
    return m_programArgs;
}

int main(int argc, char *argv[]) {
    Enact::start(argc, argv);
    return 0;
}