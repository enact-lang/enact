#ifndef ENACT_TYPE_H
#define ENACT_TYPE_H

#include "Token.h"
#include <vector>
#include <unordered_map>

// The type tree used to give expressions types.
// Here's what it looks like:

// Type
// ├── Numerical
// │   ├── Integral
// │   └── Decimal
// ├── String
// ├── Struct
// ├── Dynamic
// └── Nothing

class Type {
public:
    enum class Kind {
        PRIMITIVE,
        ARRAY,
        FUNCTION,
        TRAIT,
        STRUCT
    };
private:
    Kind m_kind;
public:
    Type(Kind kind);

    virtual Kind getKind() const;

    virtual bool operator==(const Type &type) const;
    virtual bool operator!=(const Type &type) const;

    virtual std::string toString() const;

    // Primitive type groups
    bool isNumeric() const;
    bool isBool() const;
    bool isString() const;
    bool isDynamic() const;
    bool isNothing() const;

    // Complex type groups
    bool isArray() const;
    bool isFunction() const;
    bool isTrait() const;
    bool isStruct() const;

    template <typename T>
    inline const T& as() const;

    class Primitive;
    class Array;
    class Function;
    class Trait;
    class Struct;
};

// Primitive types
// - numbers/strings/dynamic/nothing
class Type::Primitive : public Type {
public:
    enum class Kind {
        INT,
        FLOAT,
        BOOL,
        STRING,
        DYNAMIC,
        NOTHING,
    };
private:
    Kind m_kind;
public:
    Primitive(Kind kind);

    Kind getPrimitiveKind() const;
};

// Array types
class Type::Array : public Type {
    Type* m_elementType;
public:
    Array(Type* elementType);

    const Type* const getElementType() const;
};

// Function types
class Type::Function : public Type {
    Type* m_returnType;
    std::vector<Type*> m_argumentTypes;
public:
    Function(Type* returnType, std::vector<Type*> argumentTypes);

    const Type* const getReturnType() const;
    const std::vector<Type*>& getArgumentTypes() const;
};

// Trait types
class Type::Trait : public Type {
    std::string m_name;
public:
    Trait(std::string name);

    const std::string& getName() const;
};

// Struct types
class Type::Struct : public Type {
    std::string m_name;
    std::vector<Type*> m_traits;
    std::unordered_map<std::string, Type*> m_fields;
public:
    Struct(std::string name, std::vector<Type*> traits);

    const std::string& getName() const;
    const std::vector<Type*> getTraits() const;

    bool hasTrait(Type* trait) const;
};

#endif //ENACT_TYPE_H
