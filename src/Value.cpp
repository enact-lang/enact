#include "h/Value.h"

#include <sstream>

Value::Value(int value) : m_type{ValueType::INT}, m_value{.asInt = value} {}
Value::Value(double value) : m_type{ValueType::DOUBLE}, m_value{.asDouble = value} {}
Value::Value(bool value) : m_type{ValueType::BOOL}, m_value{.asBool = value} {}
Value::Value(Object* value) : m_type{ValueType::OBJECT}, m_value{.asObject = value} {}
Value::Value() : m_type{ValueType::INT}, m_value{.asInt = 0} {}

bool Value::is(ValueType type) const {
    return m_type == type;
}

bool Value::isInt() const {
    return is(ValueType::INT);
}

bool Value::isDouble() const {
    return is(ValueType::DOUBLE);
}

bool Value::isBool() const {
    return is(ValueType::BOOL);
}

bool Value::isObject() const {
    return is(ValueType::OBJECT);
}

int Value::asInt() const {
    return m_value.asInt;
}

double Value::asDouble() const {
    return m_value.asDouble;
}

bool Value::asBool() const {
    return m_value.asBool;
}

Object* Value::asObject() const {
    return m_value.asObject;
}

std::string Value::toString() const {
    std::stringstream s;

    if (isInt()) {
        s << asInt();
    } else if (isDouble()) {
        s << asDouble();
    } else if (isBool()) {
        s << (asBool() ? "true" : "false");
    } else if (isObject()) {
        s << *asObject();
    }

    return s.str();
}

std::ostream &operator<<(std::ostream &stream, const Value &value) {
    stream << value.toString();
    return stream;
}
