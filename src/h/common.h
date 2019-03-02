#ifndef ENACT_COMMON_H
#define ENACT_COMMON_H

#include <cstdint>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <vector>

typedef int index_t;
typedef uint32_t line_t;
typedef uint16_t col_t;

#define ENACT_ASSERT(expr, msg) \
        _enactAssert(expr, #expr, msg, __FILE__, __LINE__)

inline void _enactAssert(bool expr, std::string exprString, std::string msg, std::string file, int line) {
    if (!expr) {
        std::cerr << "Assertion failed:    " << msg << "\n"
                  << "Expected:    " << exprString << "\n"
                  << "Source:        " << file << ", line " << line << "\n";
        abort();
    }
}

#define ENACT_ABORT(msg) \
        _enactAbort(msg, __FILE__, __LINE__)

inline void _enactAbort(std::string msg, std::string file, int line) {
    std::cerr << "Aborted:    " << msg << "\n"
              << "Source:        " << file << ", line " << line << "\n";
    abort();
}

template <typename K, typename V>
inline std::vector<K> getMapKeys(const std::unordered_map<K, V>& map) {
    std::vector<K> keys;
    for (const auto& pair : map) {
        keys.push_back(pair.first);
    }
    return std::move(keys);
}

template <typename K, typename V>
inline std::vector<V> getMapValues(const std::unordered_map<K, V>& map) {
    std::vector<V> values;
    for (const auto& pair : map) {
        values.push_back(pair.second);
    }
    return std::move(values);
}

#endif //ENACT_COMMON_H
