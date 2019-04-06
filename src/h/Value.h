#ifndef ENACT_VALUE_H
#define ENACT_VALUE_H

#include <variant>
#include "Object.h"

class Value {
    std::variant<int, double, bool, Object*> m_value;

public:

    Value(int value);
    Value(double value);
    Value(bool value);
    Value(Object* value);

    template <typename T>
    inline bool is();

    template <typename T>
    T as();
};

template<typename T>
inline bool Value::is() {
    return std::holds_alternative<T>(m_value);
}

template<typename T>
inline T Value::as() {
    return std::get<T>(m_value);
}

#endif //ENACT_VALUE_H
