#ifndef ENACT_CONTEXT_H
#define ENACT_CONTEXT_H

#include <list>

#include "../analyser/Analyser.h"
#include "../compiler/Compiler.h"
#include "../memory/GC.h"
#include "../parser/Parser.h"
#include "../vm/VM.h"

#include "Options.h"

enum class InterpretResult {
    OK = 0,
    PARSE_ERROR,
    ANALYSIS_ERROR,
    COMPILE_ERROR,
    RUNTIME_ERROR,
};

class Context {
    InterpretResult runFile();
    std::string readFile(const std::string& filename);

    InterpretResult runRepl();

public:
    Options options;
    std::string source;

    GC gc{*this};
    VM vm{*this};
    Parser parser{*this};
    Analyser analyser{*this};

    explicit Context(Options options);
    ~Context() = default;

    InterpretResult run();

    Compiler& pushCompiler();
    void popCompiler();
    Compiler& currentCompiler();

    std::string getSourceLine(line_t line);
    void reportErrorAt(const Token& token, const std::string& msg);

private:
    std::list<Compiler> m_compilers{Compiler{*this}};
};

template <typename T, typename... Args>
T* GC::allocateObject(Args&&... args) {
    static_assert(std::is_base_of_v<Object, T>,
                  "GC::allocateObject<T>: T must derive from Object.");

    m_bytesAllocated += sizeof(T);
    if (m_bytesAllocated > m_nextRun || m_context.options.flagEnabled(Flag::DEBUG_STRESS_GC)) {
        collectGarbage();
    }

    T* object = new T{args...};

    m_objects.push_back(object);

    if (m_context.options.flagEnabled(Flag::DEBUG_LOG_GC)) {
        std::cout << static_cast<void *>(object) << ": allocated object of size " << sizeof(T) << " and type " <<
                  static_cast<int>(static_cast<Object*>(object)->m_type) << ".\n";
    }

    return object;
}

#endif //ENACT_CONTEXT_H
