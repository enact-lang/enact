#include "h/Type.h"

Type::Type(Kind kind) : m_kind{kind} {

}

Type::Kind Type::getKind() const {
    return m_kind;
}

bool Type::operator==(const Type &type) const {
    if (type.getKind() != m_kind) return false;

    switch (m_kind) {
        case Kind::PRIMITIVE: {
            auto left = this->as<Type::Primitive>();
            auto right = type.as<Type::Primitive>();
            return left.getPrimitiveKind() == right.getPrimitiveKind();
        }
        case Kind::ARRAY: {
            auto left = this->as<Type::Array>();
            auto right = type.as<Type::Array>();
            return *(left.getElementType()) == *(right.getElementType());
        }
        case Kind::FUNCTION: {
            auto left = this->as<Type::Function>();
            auto right = type.as<Type::Function>();

            if (*(left.getReturnType()) != *(right.getReturnType())) return false;
            if (left.getArgumentTypes().size() != right.getArgumentTypes().size()) return false;

            for (int i = 0; i < left.getArgumentTypes().size(); ++i) {
                if (*(left.getArgumentTypes()[i]) != *(right.getArgumentTypes()[i])) return false;
            }

            return true;
        }
        case Kind::TRAIT: {
            auto left = this->as<Type::Trait>();
            auto right = type.as<Type::Trait>();

            return left.getName() == right.getName();
        }
        case Kind::STRUCT: {
            auto left = this->as<Type::Struct>();
            auto right = type.as<Type::Struct>();

            return left.getName() == right.getName();
        }

        // Unreachable
        default: return false;
    }
}

bool Type::operator!=(const Type &type) const {
    return !(*this == type);
}

std::string Type::toString() const {
    return std::__cxx11::string();
}

bool Type::isNumeric() const {
    if (m_kind != Kind::PRIMITIVE) return false;
    auto primitiveKind = this->as<Type::Primitive>().getPrimitiveKind();
    return primitiveKind == Primitive::Kind::INT || primitiveKind == Primitive::Kind::FLOAT;
}

bool Type::isBool() const {
    if (m_kind != Kind::PRIMITIVE) return false;
    return this->as<Type::Primitive>().getPrimitiveKind() == Primitive::Kind::BOOL;
}

bool Type::isString() const {
    if (m_kind != Kind::PRIMITIVE) return false;
    return this->as<Type::Primitive>().getPrimitiveKind() == Primitive::Kind::STRING;
}

bool Type::isDynamic() const {
    if (m_kind != Kind::PRIMITIVE) return false;
    return this->as<Type::Primitive>().getPrimitiveKind() == Primitive::Kind::DYNAMIC;
}

bool Type::isNothing() const {
    if (m_kind != Kind::PRIMITIVE) return false;
    return this->as<Type::Primitive>().getPrimitiveKind() == Primitive::Kind::NOTHING;
}

bool Type::isArray() const {
    return m_kind == Kind::ARRAY;
}

bool Type::isFunction() const {
    return m_kind == Kind::FUNCTION;
}

bool Type::isTrait() const {
    return m_kind == Kind::TRAIT;
}

bool Type::isStruct() const {
    return m_kind == Kind::STRUCT;
}

template<typename T>
inline const T& Type::as() const {
    return *static_cast<const T*>(this);
}

Type::Primitive::Primitive(Type::Primitive::Kind kind) :
        Type{Type::Kind::PRIMITIVE},
        m_kind{kind} {}

Type::Primitive::Kind Type::Primitive::getPrimitiveKind() const {
    return m_kind;
}

Type::Array::Array(Type *elementType) :
        Type{Type::Kind::ARRAY},
        m_elementType{elementType} {}

const Type *const Type::Array::getElementType() const {
    return m_elementType;
}

Type::Function::Function(Type *returnType, std::vector<Type *> argumentTypes) :
        Type{Type::Kind::FUNCTION},
        m_returnType{returnType},
        m_argumentTypes{argumentTypes} {}

const Type *const Type::Function::getReturnType() const {
    return m_returnType;
}

const std::vector<Type *> &Type::Function::getArgumentTypes() const {
    return m_argumentTypes;
}

Type::Struct::Struct(std::string name, std::vector<Type *> traits) :
        Type{Type::Kind::STRUCT},
        m_name{name},
        m_traits{traits} {}

const std::string &Type::Struct::getName() const {
    return m_name;
}

const std::vector<Type *> Type::Struct::getTraits() const {
    return m_traits;
}

bool Type::Struct::hasTrait(Type *trait) const {
    for (Type* thisTrait : m_traits) {
        if (*thisTrait == *trait) return true;
    }
    return false;
}
