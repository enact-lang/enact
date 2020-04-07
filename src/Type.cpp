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
        case TypeKind::CONSTRUCTOR: {
            auto left = this->as<ConstructorType>();
            auto right = type.as<ConstructorType>();

            return *left->getStructType() == *right->getStructType();
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
        return type.as<StructType>()->hasTrait(*this);
    }
    if (type.isTrait() && this->isStruct()) {
        return this->as<StructType>()->hasTrait(*this);
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
        case PrimitiveKind::STRING: name = "String"; break;
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

FunctionType::FunctionType(Type returnType, std::vector<Type> argumentTypes, bool isNative) :
        TypeBase{TypeKind::FUNCTION},
        m_returnType{returnType},
        m_argumentTypes{argumentTypes},
        m_isNative{isNative} {}

const Type FunctionType::getReturnType() const {
    return m_returnType;
}

const std::vector<Type>& FunctionType::getArgumentTypes() const {
    return m_argumentTypes;
}

bool FunctionType::isNative() const {
    return m_isNative;
}

std::unique_ptr<Typename> FunctionType::toTypename() const {
    std::vector<std::unique_ptr<const Typename>> argumentTypenames;
    for (Type type : m_argumentTypes) {
        argumentTypenames.push_back(type->toTypename());
    }
    return std::make_unique<FunctionTypename>(m_returnType->toTypename(), std::move(argumentTypenames));
}

TraitType::TraitType(std::string name, InsertionOrderMap<std::string, Type> methods) :
        TypeBase{TypeKind::TRAIT},
        m_name{name},
        m_methods{methods} {}

const std::string& TraitType::getName() const {
    return m_name;
}

const InsertionOrderMap<std::string, Type>& TraitType::getMethods() const {
    return m_methods;
}

std::optional<Type> TraitType::getMethod(const std::string &name) const {
    return m_methods.at(name);
}

std::unique_ptr<Typename> TraitType::toTypename() const {
    return std::make_unique<BasicTypename>(m_name, Token{TokenType::IDENTIFIER, m_name, 0, 0});
}

StructType::StructType(
        std::string name,
        std::vector<std::shared_ptr<const TraitType>> traits,
        InsertionOrderMap<std::string, Type> fields,
        InsertionOrderMap<std::string, Type> methods) :
            TypeBase{TypeKind::STRUCT},
            m_name{std::move(name)},
            m_traits{std::move(traits)},
            m_fields{std::move(fields)},
            m_methods{std::move(methods)} {
}

const std::string& StructType::getName() const {
    return m_name;
}

const std::vector<std::shared_ptr<const TraitType>>& StructType::getTraits() const {
    return m_traits;
}

bool StructType::hasTrait(const TypeBase& trait) const {
    for (auto& myTrait : m_traits) {
        if (myTrait->looselyEquals(trait)) return true;
    }

    return false;
}

std::optional<size_t> StructType::findTrait(const TypeBase &trait) const {
    size_t index = 0;
    for (auto& myTrait : m_traits) {
        if (myTrait->looselyEquals(trait)) return index;
        ++index;
    }

    return {};
}

std::optional<Type> StructType::getProperty(const std::string &name) const {
    if (auto field = getField(name)) {
        return field;
    }

    return getMethod(name);
}

const InsertionOrderMap<std::string, Type>& StructType::getFields() const {
    return m_fields;
}

std::optional<Type> StructType::getField(const std::string& name) const {
    return m_fields.at(name);
}

std::optional<size_t> StructType::findField(const std::string& name) const {
    return m_fields.find(name);
}

const InsertionOrderMap<std::string, Type>& StructType::getMethods() const {
    return m_methods;
}

std::optional<Type> StructType::getMethod(const std::string& name) const {
    return m_methods.at(name);
}

std::optional<size_t> StructType::findMethod(const std::string& name) const {
    return m_methods.find(name);
}

std::unique_ptr<Typename> StructType::toTypename() const {
    return std::make_unique<BasicTypename>(m_name, Token{TokenType::IDENTIFIER, m_name, 0, 0});
}

ConstructorType::ConstructorType(std::shared_ptr<const StructType> structType, InsertionOrderMap<std::string, Type> assocProperties) :
        TypeBase{TypeKind::CONSTRUCTOR},
        m_structType{structType},
        m_assocProperties{assocProperties} {
}

std::shared_ptr<const StructType> ConstructorType::getStructType() const {
    return m_structType;
}

const InsertionOrderMap<std::string, Type>& ConstructorType::getAssocProperties() const {
    return m_assocProperties;
}

std::optional<Type> ConstructorType::getAssocProperty(const std::string &name) const {
    return m_assocProperties.at(name);
}

std::optional<size_t> ConstructorType::findAssocProperty(const std::string &name) const {
    return m_assocProperties.find(name);
}

std::unique_ptr<Typename> ConstructorType::toTypename() const {
    std::string name = m_structType->toString();
    return std::make_unique<ConstructorTypename>(
            std::make_unique<BasicTypename>(name, Token{TokenType::IDENTIFIER, name, 0, 0}));
}
