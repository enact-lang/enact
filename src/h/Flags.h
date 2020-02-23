#ifndef ENACT_FLAGS_H
#define ENACT_FLAGS_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>

enum class Flag {
    DEBUG_PRINT_AST,
    DEBUG_DISASSEMBLE_CHUNK,
    DEBUG_TRACE_EXECUTION,
    DEBUG_STRESS_GC,
    DEBUG_LOG_GC
};

class Flags {
    std::unordered_set<Flag> m_flags{};
    bool m_hadError{false};

public:
    Flags() = default;
    explicit Flags(std::unordered_set<Flag> flags);
    explicit Flags(const std::vector<std::string>& strings);

    void parseString(const std::string& string);
    void parseStrings(const std::vector<std::string>& strings);

    bool flagEnabled(Flag flag) const;
    void enableFlag(Flag flag);
    void enableFlags(std::vector<Flag> flags);

    bool hadError();

private:
    std::unordered_map<std::string, std::function<void()>> m_parseTable{
            {"--debug-print-ast",         std::bind(&Flags::enableFlag, this, Flag::DEBUG_PRINT_AST)},
            {"--debug-disassemble-chunk", std::bind(&Flags::enableFlag, this, Flag::DEBUG_DISASSEMBLE_CHUNK)},
            {"--debug-trace-execution",   std::bind(&Flags::enableFlag, this, Flag::DEBUG_TRACE_EXECUTION)},
            {"--debug-stress-gc",         std::bind(&Flags::enableFlag, this, Flag::DEBUG_STRESS_GC)},
            {"--debug-log-gc",            std::bind(&Flags::enableFlag, this, Flag::DEBUG_LOG_GC)},

            {"--debug",                   std::bind(&Flags::enableFlags, this, std::vector<Flag>{
                Flag::DEBUG_PRINT_AST,
                Flag::DEBUG_DISASSEMBLE_CHUNK,
                Flag::DEBUG_TRACE_EXECUTION,
                Flag::DEBUG_STRESS_GC,
                Flag::DEBUG_LOG_GC
            })},
    };
};

#endif //ENACT_FLAGS_H
