#include "h/Object.h"
#include "h/Value.h"
#include "h/Chunk.h"

Object* Object::m_objects = nullptr;

Object::Object(ObjectType type) : m_type{type} {
    m_next = m_objects;
    m_objects = this;
}

void Object::freeAll() {
    Object* object = m_objects;
    while (object != nullptr) {
        Object* next = object->m_next;
        delete object;
        object = next;
    }
}

bool Object::operator==(const Object &object) const {
    if (m_type != object.m_type) {
        return false;
    }

    switch (m_type) {
        case ObjectType::STRING:
            return this->as<StringObject>()->asStdString() == object.as<StringObject>()->asStdString();
        case ObjectType::ARRAY:
            return this->as<ArrayObject>()->asVector() == object.as<ArrayObject>()->asVector();
    }
}

std::ostream& operator<<(std::ostream& stream, const Object& object) {
    stream << object.toString();
    return stream;
}

StringObject::StringObject(std::string data) : Object{ObjectType::STRING}, m_data{std::move(data)} {

}

const std::string& StringObject::asStdString() const {
    return m_data;
}

std::string StringObject::toString() const {
    return asStdString();
}

Type StringObject::getType() const {
    return STRING_TYPE;
}

ArrayObject::ArrayObject() : Object{ObjectType::ARRAY}, m_vector{} {
}

ArrayObject::ArrayObject(std::vector<Value> vector) : Object{ObjectType::ARRAY}, m_vector{std::move(vector)} {
}

std::optional<Value> ArrayObject::at(size_t index) const {
    if (index >= m_vector.size()) {
        return {};
    }

    return {m_vector[index]};
}

const std::vector<Value>& ArrayObject::asVector() const {
    return m_vector;
}

std::string ArrayObject::toString() const {
    std::string output{"["};
    std::string separator{};
    for (const Value& item : asVector()) {
        output += separator;
        output += item.toString();
        separator = ", ";
    }
    return output;
}

Type ArrayObject::getType() const {
    Type elementType = DYNAMIC_TYPE;

    for (const auto& element : m_vector) {
        elementType = element.getType();
    }

    return std::make_shared<ArrayType>(elementType);
}

FunctionObject::FunctionObject(Type type, Chunk chunk, std::string name) :
        Object{ObjectType::FUNCTION}, m_type{type}, m_chunk{std::move(chunk)}, m_name{std::move(name)} {
    ENACT_ASSERT(m_type->as<FunctionType>()->getArgumentTypes().size() <= UINT8_MAX,
            "FunctionObject::FunctionObject: Exceeded maximum of 255 parameters.");
}

const Chunk& FunctionObject::getChunk() const {
    return m_chunk;
}

const std::string& FunctionObject::getName() const {
    return m_name;
}

std::string FunctionObject::toString() const {
    // Check if this is the global function
    if (m_name.empty()) {
        return "<script>";
    } else {
        return "<fun " + m_name + ">";
    }
}

Type FunctionObject::getType() const {
    return m_type;
}



