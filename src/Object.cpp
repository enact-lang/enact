#include <sstream>
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

UpvalueObject::UpvalueObject(uint32_t location) : Object{ObjectType::UPVALUE}, m_location{location} {

}

uint32_t UpvalueObject::getLocation() {
    return m_location;
}

UpvalueObject* UpvalueObject::getNext() {
    return m_next;
}

void UpvalueObject::setNext(UpvalueObject *next) {
    m_next = next;
}

bool UpvalueObject::isClosed() const {
    return m_isClosed;
}

Value UpvalueObject::getClosed() const {
    return m_closed;
}

void UpvalueObject::setClosed(Value value) {
    m_isClosed = true;
    m_closed = value;
}

std::string UpvalueObject::toString() const {
    return "upvalue";
}

Type UpvalueObject::getType() const {
    return NOTHING_TYPE;
}


ClosureObject::ClosureObject(FunctionObject *function) : Object{ObjectType::CLOSURE}, m_function{function}, m_upvalues{function->getUpvalueCount()} {
}

FunctionObject* ClosureObject::getFunction() {
    return m_function;
}

std::vector<UpvalueObject*>& ClosureObject::getUpvalues() {
    return m_upvalues;
}

std::string ClosureObject::toString() const {
    return m_function->toString();
}

Type ClosureObject::getType() const {
    return m_function->getType();
}

FunctionObject::FunctionObject(Type type, Chunk chunk, std::string name) :
        Object{ObjectType::FUNCTION}, m_type{type}, m_chunk{std::move(chunk)}, m_name{std::move(name)} {
}

Chunk& FunctionObject::getChunk() {
    return m_chunk;
}

const std::string& FunctionObject::getName() const {
    return m_name;
}

uint32_t& FunctionObject::getUpvalueCount() {
    return m_upvalueCount;
}

std::string FunctionObject::toString() const {
    // Check if this is the global function
    if (m_name.empty()) {
        return "<script>";
    } else {
        std::stringstream ret;
        ret << "<" << m_type->toString() << ">";
        return ret.str();
    }
}

Type FunctionObject::getType() const {
    return m_type;
}

NativeObject::NativeObject(Type type, NativeFn function) : Object{ObjectType::NATIVE}, m_type{type}, m_function{function} {
}

NativeFn NativeObject::getFunction() {
    return m_function;
}

std::string NativeObject::toString() const {
    std::stringstream ret;
    ret << "<native " << m_type->toString() << ">";
    return ret.str();
}

Type NativeObject::getType() const {
    return m_type;
}