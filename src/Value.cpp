#include "h/Value.h"

Value::Value(int value) : m_value{value} {}
Value::Value(double value) : m_value{value} {}
Value::Value(bool value) : m_value{value} {}
Value::Value(Object* value) : m_value{value} {}
