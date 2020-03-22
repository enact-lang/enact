#include "h/Type.h"

#include "ast/Stmt.h"

#include <algorithm>
#include <sstream>

const Type INT_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::INT);
const Type FLOAT_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::FLOAT);
const Type BOOL_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::BOOL);
const Type STRING_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::STRING);
const Type DYNAMIC_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::DYNAMIC);
const Type NOTHING_TYPE = std::make_shared<PrimitiveType>(PrimitiveKind::NOTHING);

TypeBase::TypeBase(TypeKind kind) :
        m_kind{kind} {}

TypeKind TypeBase::getKind() const {
    return m_kind;
}

bool TypeBase::operator==(const TypeBase &type) const {
    if (type.getKind() != m_kind) return false;

    switch (m_kind) {
        case TypeKind::PRIMITIVE: {
            auto left = this->as<PrimitiveType>();
            auto right = type.as<PrimitiveType>();
            return left->getPrimitiveKind() == right->getPrimitiveKind();
        }
        case TypeKind::ARRAY: {
            auto left = this->as<ArrayType>();
            auto right = type.as<ArrayType>();
            return *(left->getElementType()) == *(right->getElementType());
        }
        case TypeKind::FUNCTION: {
            auto left = this->as<FunctionType>();
            auto right = type.as<FunctionType>();

            if (*(left->getReturnType()) != *(right->getReturnType())) return false;
            if (left->getArgumentTypes().size() != right->getArgumentTypes().size()) return false;

            for (int i = 0; i < left->getArgumentTypes().size(); ++i) {
                if (*(left->getArgumentTypes()[i]) != *(right->getArgumentTypes()[i])) return false;
            }

            return true;
        }
        case TypeKind::TRAIT: {
            auto left = this->as<TraitType>();
            auto right = type.as<TraitType>();

            return left->getName() == right->getName();
        }
        case TypeKind::STRUCT: {
            auto left = this->as<StructType>();
            auto right = type.as<StructType>();

            return left->getName() == right->getName();
        }

            // Unreachable
        default: return false;
    }
}

bool TypeBase::operator!=(const TypeBase &type) const {
    return !(*this == type);
}

bool TypeBase::looselyEquals(const TypeBase &type) const {
    if (this->isArray() && type.isArray()) {
        return this->as<ArrayType>()->getElementType()->looselyEquals(*type.as<ArrayType>()->getElementType());
    }
    if (this->isTrait() && type.isStruct()) {
        return type.as<StructType>()->getTrait(*this) != std::nullopt;
    }
    if (type.isTrait() && this->isStruct()) {
        return this->as<StructType>()->getTrait(type) != std::nullopt;
    }

    return this->isDynamic() || type.isDynamic() || (this->isInt() && type.isNumeric()) || *this == type;
}

std::string TypeBase::toString() const {
    return toTypename()->name();
}

bool TypeBase::isPrimitive() const {
    return m_kind == TypeKind::PRIMITIVE && !isDynamic();
}

bool TypeBase::isInt() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::INT;
}

bool TypeBase::isFloat() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::FLOAT;
}

bool TypeBase::isNumeric() const {
    return isInt() || isFloat();
}

bool TypeBase::isBool() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::BOOL;
}

bool TypeBase::isString() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::STRING;
}

bool TypeBase::isDynamic() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::DYNAMIC;
}

bool TypeBase::isNothing() const {
    return m_kind == TypeKind::PRIMITIVE &&
           this->as<PrimitiveType>()->getPrimitiveKind() == PrimitiveKind::NOTHING;
}

bool TypeBase::maybePrimitive() const {
    return m_kind == TypeKind::PRIMITIVE;
}

bool TypeBase::maybeInt() const {
    return isInt() || isDynamic();
}

bool TypeBase::maybeFloat() const {
    return isFloat() || isDynamic();
}

bool TypeBase::maybeNumeric() const {
    return isNumeric() || isDynamic();
}

bool TypeBase::maybeBool() const {
    return isBool() || isDynamic();
}

bool TypeBase::maybeString() const {
    return isString() || isDynamic();
}

bool TypeBase::isArray() const {
    return m_kind == TypeKind::ARRAY;
}

bool TypeBase::isFunction() const {
    return m_kind == TypeKind::FUNCTION;
}

bool TypeBase::isTrait() const {
    return m_kind == TypeKind::TRAIT;
}

bool TypeBase::isStruct() const {
    return m_kind == TypeKind::STRUCT;
}

bool TypeBase::isConstructor() const {
    return m_kind == TypeKind::CONSTRUCTOR;
}

bool TypeBase::maybeArray() const {
    return isArray() || isDynamic();
}

bool TypeBase::maybeFunction() const {
    return isFunction() || isDynamic();
}

bool TypeBase::maybeTrait() const {
    return isTrait() || isDynamic();
}

bool TypeBase::maybeStruct() const {
    return isStruct() || isDynamic();
}


// Primitive types
PrimitiveType::PrimitiveType(PrimitiveKind kind) :
        TypeBase{TypeKind::PRIMITIVE},
        m_kind{kind} {}

PrimitiveKind PrimitiveType::getPrimitiveKind() const {
    return m_kind;
}

std::unique_ptr<Typename> PrimitiveType::toTypename() const {
    std::string name;
    switch (m_kind) {
        case PrimitiveKind::INT: name = "int"; break;
        case PrimitiveKind::FLOAT: name = "float"; break;
        case PrimitiveKind::BOOL: name = "bool"; break;
        case PrimitiveKind::STRING: name = "string"; break;
        case PrimitiveKind::DYNAMIC: name = "any"; break;
        case PrimitiveKind::NOTHING: name = "nothing"; break;
    }
    return std::make_unique<BasicTypename>(Token{TokenType::IDENTIFIER, name, 0, 0});
}

ArrayType::ArrayType(Type elementType) :
        TypeBase{TypeKind::ARRAY},
        m_elementType{elementType} {}

const Type ArrayType::getElementType() const {
    return m_elementType;
}

std::unique_ptr<Typename> ArrayType::toTypename() const {
    return std::make_unique<ArrayTypename>(m_elementType->toTypename());
}

FunctionType::FunctionType(Type returnType, std::vector<Type> argumentTypes) :
        TypeBase{TypeKind::FUNCTION},
        m_returnType{returnType},
        m_argumentTypes{argumentTypes} {}

const Type FunctionType::getReturnType() const {
    return m_returnType;
}

const std::vector<Type>& FunctionType::getArgumentTypes() const {
    return m_argumentTypes;
}

std::unique_ptr<Typename> FunctionType::toTypename() const {
    std::vector<std::unique_ptr<const Typename>> argumentTypenames;
    for (Type type : m_argumentTypes) {
        argumentTypenames.push_back(type->toTypename());
    }
    return std::make_unique<FunctionTypename>(m_returnType->toTypename(), std::move(argumentTypenames));
}

TraitType::TraitType(std::string name, std::unordered_map<std::string, Type> methods) :
        TypeBase{TypeKind::TRAIT},
        m_name{name},
        m_methods{methods} {}

const std::string& TraitType::getName() const {
    return m_name;
}

const std::unordered_map<std::string, Type>& TraitType::getMethods() const {
    return m_methods;
}

std::optional<Type> TraitType::getMethod(const std::string &name) const {
    if (m_methods.count(name) > 0) {
        return std::make_optional(m_methods.at(name));
    }

    return {};
}

std::unique_ptr<Typename> TraitType::toTypename() const {
    return std::make_unique<BasicTypename>(m_name, Token{TokenType::IDENTIFIER, m_name, 0, 0});
}

StructType::StructType(std::string name, std::vector<Type> traits,
                       std::unordered_map<std::string, Type> fields,
                       std::unordered_map<std::string, Type> methods,
                       std::unordered_map<std::string, Type> assocFunctions) :
        TypeBase{TypeKind::STRUCT},
        m_name{name},
        m_traits{traits},
        m_fields{fields},
        m_methods{methods},
        m_assocFunctions{assocFunctions} {}

const std::string& StructType::getName() const {
    return m_name;
}

const std::vector<Type>& StructType::getTraits() const {
    return m_traits;
}

const std::unordered_map<std::string, Type>& StructType::getFields() const {
    return m_fields;
}

const std::unordered_map<std::string, Type>& StructType::getMethods() const {
    return m_methods;
}

const std::unordered_map<std::string, Type>& StructType::getAssocFunctions() const {
    return m_assocFunctions;
}

std::optional<Type> StructType::getTrait(const TypeBase& trait) const {
    for (const Type& myTrait : m_traits) {
        if (*myTrait == trait) return myTrait;
    }
    return {};
}

std::optional<Type> StructType::getField(const std::string &name) const {
    if (m_fields.count(name) > 0) {
        return std::make_optional(m_fields.at(name));
    }
    return {};
}

std::optional<Type> StructType::getMethod(const std::string &name) const {
    if (m_methods.count(name) > 0) {
        return std::make_optional(m_methods.at(name));
    }

    return {};
}

std::optional<Type> StructType::getFieldOrMethod(const std::string &name) const {
    if (std::optional<Type> type; type = getField(name)) {
        return type;
    }
    return getMethod(name);
}

std::optional<Type> StructType::getAssocFunction(const std::string &name) const {
    if (m_assocFunctions.count(name) > 0) {
        return std::make_optional(m_assocFunctions.at(name));
    }
    return {};
}

std::unique_ptr<Typename> StructType::toTypename() const {
    return std::make_unique<BasicTypename>(m_name, Token{TokenType::IDENTIFIER, m_name, 0, 0});
}

ConstructorType::ConstructorType(StructType structType) :
        TypeBase{TypeKind::CONSTRUCTOR},
        m_structType{structType},
        m_functionType{FunctionType(std::make_shared<StructType>(structType), getMapValues(structType.getFields()))} {
}

const StructType &ConstructorType::getStructType() const {
    return m_structType;
}

const FunctionType &ConstructorType::getFunctionType() const {
    return m_functionType;
}

std::unique_ptr<Typename> ConstructorType::toTypename() const {
    return m_functionType.toTypename();
}
