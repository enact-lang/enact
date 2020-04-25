#include <sstream>

#include "../bytecode/Chunk.h"
#include "../memory/GC.h"
#include "../vm/VM.h"

#include "Object.h"
#include "Value.h"

#ifdef DEBUG_LOG_GC
#include <iostream>
#include "h/Chunk.h"
#endif

Object::Object(ObjectType type) : m_type{type} {
}

Object::~Object() {
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

void Object::mark() {
    m_isMarked = true;
}

void Object::unmark() {
    m_isMarked = false;
}

bool Object::isMarked() {
    return m_isMarked;
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

StringObject* StringObject::clone() const {
    return new StringObject(*this);
}

size_t StringObject::size() const {
    return sizeof(StringObject);
}

ArrayObject::ArrayObject(Type type) : Object{ObjectType::ARRAY}, m_type{type}, m_vector{} {
}

ArrayObject::ArrayObject(size_t length, Type type) : Object{ObjectType::ARRAY}, m_vector{length}, m_type{type} {
}

ArrayObject::ArrayObject(std::vector<Value> vector, Type type) : Object{ObjectType::ARRAY}, m_vector{std::move(vector)}, m_type{type} {
}

size_t ArrayObject::length() const {
    return m_vector.size();
}

Value& ArrayObject::at(size_t index) {
    return m_vector[index];
}

const Value& ArrayObject::at(size_t index) const {
    return m_vector[index];
}

void ArrayObject::append(Value value) {
    m_vector.push_back(value);
}

const std::vector<Value>& ArrayObject::asVector() const {
    return m_vector;
}

std::string ArrayObject::toString() const {
    std::stringstream output{};
    std::string separator{};

    output << "[";
    for (const Value& item : asVector()) {
        output << separator;
        output << item.toString();
        separator = ", ";
    }
    output << "]";

    return output.str();
}

Type ArrayObject::getType() const {
    return m_type;
}

ArrayObject* ArrayObject::clone() const {
    return new ArrayObject(*this);
}

size_t ArrayObject::size() const {
    return sizeof(ArrayObject);
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

UpvalueObject* UpvalueObject::clone() const {
    return new UpvalueObject(*this);
}

size_t UpvalueObject::size() const {
    return sizeof(UpvalueObject);
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

ClosureObject* ClosureObject::clone() const {
    return new ClosureObject(*this);
}

size_t ClosureObject::size() const {
    return sizeof(ClosureObject);
}

StructObject::StructObject(std::shared_ptr<const ConstructorType> constructorType, std::vector<ClosureObject*> methods, std::vector<Value> assocs) :
        Object{ObjectType::STRUCT},
        m_constructorType{std::move(constructorType)},
        m_methods{std::move(methods)},
        m_assocs{std::move(assocs)} {
}

const std::string& StructObject::getName() const {
    return m_constructorType->getStructType()->getName();
}

std::vector<ClosureObject*>& StructObject::methods() {
    return m_methods;
}

ClosureObject* StructObject::method(uint32_t index) {
    return m_methods[index];
}

std::optional<ClosureObject*> StructObject::methodNamed(const std::string& name) {
    if (auto index = m_constructorType->getStructType()->findMethod(name)) {
        return m_methods[*index];
    }
    return {};
}

std::vector<Value>& StructObject::assocs() {
    return m_assocs;
}

Value& StructObject::assoc(uint32_t index) {
    return m_assocs[index];
}

std::optional<std::reference_wrapper<Value>> StructObject::assocNamed(const std::string &name) {
    if (auto index = m_constructorType->findAssocProperty(name)) {
        return m_assocs[*index];
    }
    return {};
}

std::string StructObject::toString() const {
    return "<" + m_constructorType->toString() + ">";
}

Type StructObject::getType() const {
    return m_constructorType;
}

StructObject* StructObject::clone() const {
    return new StructObject(*this);
}

size_t StructObject::size() const {
    return sizeof(StructObject);
}

InstanceObject::InstanceObject(StructObject *struct_, std::vector<Value> fields) :
        Object{ObjectType::INSTANCE},
        m_struct{struct_},
        m_fields{std::move(fields)} {
}

StructObject* InstanceObject::getStruct() {
    return m_struct;
}

std::vector<Value>& InstanceObject::fields() {
    return m_fields;
}

Value &InstanceObject::field(uint32_t index) {
    return m_fields[index];
}

std::optional<std::reference_wrapper<Value>> InstanceObject::fieldNamed(const std::string &name) {
    auto structType = getType()->as<StructType>();
    if (std::optional<size_t> index = structType->findField(name)) {
        return m_fields[*index];
    }

    return {};
}

std::string InstanceObject::toString() const {
    return "<" + getType()->toString() + " instance>";
}

Type InstanceObject::getType() const {
    return m_struct->getType()->as<ConstructorType>()->getStructType();
}

StructObject *InstanceObject::clone() const {
    return nullptr;
}

size_t InstanceObject::size() const {
    return 0;
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

FunctionObject* FunctionObject::clone() const {
    return new FunctionObject(*this);
}

size_t FunctionObject::size() const {
    return sizeof(FunctionObject);
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

NativeObject* NativeObject::clone() const {
    return new NativeObject(*this);
}

size_t NativeObject::size() const {
    return sizeof(NativeObject);
}

TypeObject::TypeObject(Type containedType) : Object{ObjectType::TYPE}, m_containedType{containedType} {
}

Type TypeObject::getContainedType() {
    return m_containedType;
}

std::string TypeObject::toString() const {
    return m_containedType->toString();
}

Type TypeObject::getType() const {
    return NOTHING_TYPE;
}

TypeObject* TypeObject::clone() const {
    return new TypeObject(*this);
}

size_t TypeObject::size() const {
    return sizeof(TypeObject);
}

BoundMethodObject::BoundMethodObject(Value receiver, ClosureObject* method) :
        Object{ObjectType::BOUND_METHOD},
        m_receiver{receiver},
        m_method{method} {
}

Value BoundMethodObject::receiver() {
    return m_receiver;
}

ClosureObject* BoundMethodObject::method() {
    return m_method;
}

std::string BoundMethodObject::toString() const {
    return m_method->toString();
}

Type BoundMethodObject::getType() const {
    return m_method->getType();
}

BoundMethodObject* BoundMethodObject::clone() const {
    return new BoundMethodObject(*this);
}

size_t BoundMethodObject::size() const {
    return sizeof(BoundMethodObject);
}
