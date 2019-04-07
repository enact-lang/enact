#include "h/Value.h"

#include <sstream>

Value::Value(int value) : m_value{value} {}
Value::Value(double value) : m_value{value} {}
Value::Value(bool value) : m_value{value} {}
Value::Value(Object* value) : m_value{value} {}

std::string Value::toString() const {
    std::stringstream s;

    if (is<int>()) {
        s << as<int>();
    } else if (is<double>()) {
        s << as<double>();
    } else if (is<bool>()) {
        s << (as<bool>() ? "true" : "false");
    } else if (is<Object*>()) {
        s << *as<Object*>();
    }

    return s.str();
}

std::ostream &operator<<(std::ostream &stream, const Value &value) {
    stream << value.toString();
    return stream;
}
