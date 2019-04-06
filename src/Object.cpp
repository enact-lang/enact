#include "h/Object.h"

Object::Object(ObjectType type) : m_type{type} {}

StringObject::StringObject(std::string data) : Object{ObjectType::STRING}, m_data{std::move(data)} {

}

const std::string &StringObject::asStdString() {
    return m_data;
}
