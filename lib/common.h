#ifndef ENACT_COMMON_H
#define ENACT_COMMON_H

#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef DEBUG
#define DEBUG_ASSERTIONS_ENABLED
#endif

#ifdef DEBUG_ASSERTIONS_ENABLED
#define ENACT_ASSERT(expr, msg) \
        _assert(expr, #expr, msg, __FILE__, __LINE__)
#else
#define ENACT_ASSERT(expr, msg)
#endif

#define ENACT_ABORT(msg) \
        _abort(msg, __FILE__, __LINE__)

namespace enact {
    inline void _assert(bool expr, std::string exprString, std::string msg, std::string file, int line) {
        if (!expr) {
            std::cerr << "Assertion failed: " << msg << "\n"
                      << "Expected:         " << exprString << "\n"
                      << "Source:           " << file << ", line " << line << "\n";
            abort();
        }
    }

    inline void _abort(std::string msg, std::string file, int line) {
        std::cerr << "Aborted:    " << msg << "\n"
                  << "Source:        " << file << ", line " << line << "\n";
        abort();
    }

    typedef int index_t;
    typedef uint32_t line_t;
    typedef uint16_t col_t;
}

#endif //ENACT_COMMON_H
