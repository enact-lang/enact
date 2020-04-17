#include <fstream>
#include <sstream>

#include "../AstPrinter.h"

#include "Context.h"

Context::Context(Options options) : options{std::move(options)} {
}

InterpretResult Context::run() {
    if (!options.getFilename().empty()) {
        return runFile();
    } else {
        return runRepl();
    }
}

InterpretResult Context::runFile() {
    source = readFile(options.getFilename());

    std::vector<std::unique_ptr<Stmt>> ast = parser.parse();
    if (parser.hadError()) return InterpretResult::PARSE_ERROR;

    ast = analyser.analyse(std::move(ast));
    if (analyser.hadError()) return InterpretResult::ANALYSIS_ERROR;

    if (options.flagEnabled(Flag::DEBUG_PRINT_AST)) {
        AstPrinter printer{};
        for (auto& stmt : ast) {
            printer.print(*stmt);
        }
    }

    FunctionObject* script = currentCompiler().compileProgram(std::move(ast));
    if (currentCompiler().hadError()) return InterpretResult::COMPILE_ERROR;

    if (options.flagEnabled(Flag::DEBUG_DISASSEMBLE_CHUNK)) {
        std::cout << script->getChunk().disassemble();
    }

    return vm.run(script);
}

std::string Context::readFile(const std::string& filename) {
    // Get the file contents.
    std::ifstream file{filename};

    // Check that the file opened successfully
    if (!file.is_open()) {
        std::cerr << "[enact] Error: Unable to read file '" + filename + "'.";
        std::exit(65);
    }

    std::stringstream fileContents;
    std::string currentLine;

    while (std::getline(file, currentLine)) {
        fileContents << currentLine << "\n";
    }

    return fileContents.str();
}

InterpretResult Context::runRepl() {
    currentCompiler().startRepl();

    while (true) {
        std::cout << "enact > ";

        std::getline(std::cin, source);
        source += "\n";

        std::vector<std::unique_ptr<Stmt>> ast = parser.parse();
        if (parser.hadError()) return InterpretResult::PARSE_ERROR;

        ast = analyser.analyse(std::move(ast));
        if (analyser.hadError()) return InterpretResult::ANALYSIS_ERROR;

        if (options.flagEnabled(Flag::DEBUG_PRINT_AST)) {
            AstPrinter printer{};
            for (auto& stmt : ast) {
                printer.print(*stmt);
            }
        }

        FunctionObject* script = currentCompiler().compilePart(std::move(ast));
        if (currentCompiler().hadError()) return InterpretResult::COMPILE_ERROR;

        if (options.flagEnabled(Flag::DEBUG_DISASSEMBLE_CHUNK)) {
            std::cout << script->getChunk().disassemble();
        }

        vm.run(script);
    }

    vm.run(currentCompiler().endRepl());
}

Compiler& Context::pushCompiler() {
    m_compilers.emplace_back(*this, &m_compilers.back());
    return m_compilers.back();
}

void Context::popCompiler() {
    m_compilers.pop_back();
}

Compiler& Context::currentCompiler() {
    return m_compilers.back();
}

std::string Context::getSourceLine(line_t line) {
    std::istringstream stream{source};
    line_t lineNumber{1};
    std::string lineContents;

    while (std::getline(stream, lineContents) && lineNumber < line) {
        ++lineNumber;
    }

    return lineContents;
}

void Context::reportErrorAt(const Token &token, const std::string &msg) {
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