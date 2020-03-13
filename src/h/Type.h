#ifndef ENACT_TYPE_H
#define ENACT_TYPE_H

#include "Token.h"
#include <vector>
#include <unordered_map>
#include <optional>

/*#define INT_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::INT)
#define FLOAT_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::FLOAT)
#define BOOL_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::BOOL)
#define STRING_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::STRING)
#define DYNAMIC_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::DYNAMIC)
#define NOTHING_TYPE std::make_shared<PrimitiveType>(PrimitiveKind::NOTHING)*/

// Forward declarations from "../ast/Stmt.h":
class FunctionStmt;
struct NamedType;

// The type tree used to give expressions types.
// Here's what it looks like:

// Type
// ├── Numeric
// │   ├── Integral
// │   └── Decimal
// ├── String
// ├── Struct
// ├── Dynamic
// └── Nothing

class TypeBase;

// Type is just a managed pointer to TypeBase that allows for polymorphism.
typedef std::shared_ptr<TypeBase> Type;

extern const Type INT_TYPE;
extern const Type FLOAT_TYPE;
extern const Type BOOL_TYPE;
extern const Type STRING_TYPE;
extern const Type DYNAMIC_TYPE;
extern const Type NOTHING_TYPE;

enum class TypeKind {
    PRIMITIVE,
    ARRAY,
    FUNCTION,
    TRAIT,
    STRUCT,
    CONSTRUCTOR
};

class TypeBase {
private:
    TypeKind m_kind;
public:
    TypeBase(TypeKind kind);
    virtual ~TypeBase() = default;

    virtual TypeKind getKind() const;

    // Strict equality comparison - the types must be exactly the same
    virtual bool operator==(const TypeBase &type) const;
    virtual bool operator!=(const TypeBase &type) const;

    // Loose equality comparison - the types must be exactly the same,
    // dynamic, or convertible to each other.
    virtual bool looselyEquals(const TypeBase &type) const;

    virtual std::string toString() const;

    // Primitive type groups
    bool isPrimitive() const;
    bool isNumeric() const;
    bool isInt() const;
    bool isFloat() const;
    bool isBool() const;
    bool isString() const;
    bool isDynamic() const;
    bool isNothing() const;

    bool maybePrimitive() const;
    bool maybeNumeric() const;
    bool maybeInt() const;
    bool maybeFloat() const;
    bool maybeBool() const;
    bool maybeString() const;

    // Complex type groups
    bool isArray() const;
    bool isFunction() const;
    bool isTrait() const;
    bool isStruct() const;
    bool isConstructor() const;

    bool maybeArray() const;
    bool maybeFunction() const;
    bool maybeTrait() const;
    bool maybeStruct() const;

    template<typename T>
    inline const T* as() const {
        static_assert(std::is_base_of_v<TypeBase, T>,
                "TypeBase::as<T>: T must derive from TypeBase.");
        return static_cast<const T*>(this);
    }
};

// Primitive types
// - numbers/strings/dynamic/nothing
enum class PrimitiveKind {
    INT,
    FLOAT,
    BOOL,
    STRING,
    DYNAMIC,
    NOTHING,
};

class PrimitiveType : public TypeBase {
private:
    PrimitiveKind m_kind;
public:
    PrimitiveType(PrimitiveKind kind);
    ~PrimitiveType() override = default;

    PrimitiveKind getPrimitiveKind() const;
};

// Array types
class ArrayType : public TypeBase {
    Type m_elementType;
public:
    ArrayType(Type elementType);
    ~ArrayType() override = default;

    const Type getElementType() const;
};

// Function types
class FunctionType : public TypeBase {
    Type m_returnType;
    std::vector<Type> m_argumentTypes;
public:
    FunctionType(Type returnType, std::vector<Type> argumentTypes);
    ~FunctionType() override = default;

    const Type getReturnType() const;
    const std::vector<Type>& getArgumentTypes() const;
};


// These two user-defined types (traits and structs) must store
// a pointer to their original declaration AST node.

// Trait types
class TraitType : public TypeBase {
    std::string m_name;
    std::unordered_map<std::string, Type> m_methods;
public:
    TraitType(std::string name, std::unordered_map<std::string, Type> methods);
    ~TraitType() override = default;

    const std::string& getName() const;

    const std::unordered_map<std::string, Type>& getMethods() const;
    std::optional<Type> getMethod(const std::string& name) const;
};

// Struct types
class StructType : public TypeBase {
    std::string m_name;
    std::vector<Type> m_traits;

    std::unordered_map<std::string, Type> m_fields;
    std::unordered_map<std::string, Type> m_methods;
    std::unordered_map<std::string, Type> m_assocFunctions;

public:
    StructType(std::string name, std::vector<Type> traits, std::unordered_map<std::string, Type> fields,
               std::unordered_map<std::string, Type> methods, std::unordered_map<std::string, Type> assocFunctions);
    ~StructType() override = default;

    const std::string& getName() const;

    const std::vector<Type>& getTraits() const;
    const std::unordered_map<std::string, Type>& getFields() const;
    const std::unordered_map<std::string, Type>& getMethods() const;
    const std::unordered_map<std::string, Type>& getAssocFunctions() const;

    std::optional<Type> getTrait(const TypeBase& trait) const;

    std::optional<Type> getField(const std::string& name) const;
    std::optional<Type> getMethod(const std::string& name) const;
    std::optional<Type> getFieldOrMethod(const std::string& name) const;

    std::optional<Type> getAssocFunction(const std::string& name) const;
};

// Struct constructor types
class ConstructorType : public TypeBase {
    StructType m_structType;
    FunctionType m_functionType;
public:
    ConstructorType(StructType structType);
    ~ConstructorType() override = default;

    const StructType& getStructType() const;
    const FunctionType& getFunctionType() const;
};

#endif //ENACT_TYPE_H
