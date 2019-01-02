#ifndef ENACT_COMMON_H
#define ENACT_COMMON_H

#include <cstdint>
#include <memory>

typedef int index_t;
typedef uint32_t line_t;
typedef uint16_t col_t;

template <class T>
using Sp = std::shared_ptr<T>;

template <class T>
using Up = std::unique_ptr<T>;

#endif //ENACT_COMMON_H
