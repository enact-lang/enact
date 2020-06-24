#ifndef ENACT_COMPILECONTEXT_H
#define ENACT_COMPILECONTEXT_H

#include <list>

#include "../parser/Parser.h"

#include "Options.h"

namespace enact {
    enum class CompileResult {
        OK = 0,
        PARSE_ERROR,
        ANALYSIS_ERROR,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    class CompileContext {
    public:
        explicit CompileContext(Options options);
        ~CompileContext() = default;

        CompileResult compile(std::string source);

        const Options& getOptions() const { return m_options; }

        std::string getSourceLine(line_t line);
        void reportErrorAt(const Token &token, const std::string &msg);

    private:
        std::string m_source;
        Options m_options;

        Parser parser{*this};
    };
}

#endif //ENACT_COMPILECONTEXT_H
