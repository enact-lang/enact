#include <sstream>

#include "Object.h"
#include "Value.h"

Value::Value(int value) : m_type{ValueType::INT}, m_value{.asInt = value} {}
Value::Value(double value) : m_type{ValueType::DOUBLE}, m_value{.asDouble = value} {}
Value::Value(bool value) : m_type{ValueType::BOOL}, m_value{.asBool = value} {}
Value::Value(Object* value) : m_type{ValueType::OBJECT}, m_value{.asObject = value} {}
Value::Value() : m_type{ValueType::NIL}, m_value{.asInt = 0} {}

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

bool Value::isNil() const {
    return is(ValueType::NIL);
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

bool Value::operator==(const Value &value) const {
    if (m_type != value.m_type) {
        return false;
    }

    switch (m_type) {
        case ValueType::INT: return this->asInt() == value.asInt();
        case ValueType::DOUBLE: return this->asDouble() == value.asDouble();
        case ValueType::BOOL: return this->asBool() == value.asBool();
        case ValueType::NIL: return true;
        case ValueType::OBJECT: return *this->asObject() == *value.asObject();
    }
}

Type Value::getType() const {
    switch (m_type) {
        case ValueType::INT: return INT_TYPE;
        case ValueType::DOUBLE: return FLOAT_TYPE;
        case ValueType::BOOL: return BOOL_TYPE;
        case ValueType::NIL: return NOTHING_TYPE;
        case ValueType::OBJECT: return asObject()->getType();
    }
}

std::string Value::toString() const {
    std::stringstream s;

    if (isInt()) {
        s << asInt();
    } else if (isDouble()) {
        s << asDouble();
    } else if (isBool()) {
        s << (asBool() ? "true" : "false");
    } else if (isNil()) {
        s << "nil";
    } else if (isObject()) {
            s << *asObject();
    }

    return s.str();
}

std::ostream &operator<<(std::ostream &stream, const Value &value) {
    stream << value.toString();
    return stream;
}
