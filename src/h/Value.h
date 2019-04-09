#ifndef ENACT_VALUE_H
#define ENACT_VALUE_H

#include <variant>
#include "Object.h"
#include "Type.h"

enum class ValueType {
    INT,
    DOUBLE,
    BOOL,
    OBJECT,
    NIL,
};

class Value {
    ValueType m_type;

    union {
        int asInt;
        double asDouble;
        bool asBool;
        Object* asObject;
    } m_value;

public:
    explicit Value(int value);
    explicit Value(double value);
    explicit Value(bool value);
    explicit Value(Object* value);
    explicit Value();

    bool is(ValueType type) const;

    bool isInt() const;
    bool isDouble() const;
    bool isBool() const;
    bool isObject() const;

    int asInt() const;
    double asDouble() const;
    bool asBool() const;
    Object* asObject() const;

    Type getType() const;

    std::string toString() const;
};

std::ostream& operator<<(std::ostream& stream, const Value& value);

#endif //ENACT_VALUE_H
