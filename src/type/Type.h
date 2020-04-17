#ifndef ENACT_TYPE_H
#define ENACT_TYPE_H

#include <optional>
#include <vector>

#include "../InsertionOrderMap.h"
#include "../parser/Token.h"
#include "../parser/Typename.h"

// Forward declarations from "../ast/Stmt.h":
class FunctionStmt;

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
typedef std::shared_ptr<const TypeBase> Type;

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

    virtual std::unique_ptr<Typename> toTypename() const = 0;
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

    std::unique_ptr<Typename> toTypename() const override;
};

// Array types
class ArrayType : public TypeBase {
    Type m_elementType;
public:
    ArrayType(Type elementType);
    ~ArrayType() override = default;

    const Type getElementType() const;

    std::unique_ptr<Typename> toTypename() const override;
};

// Function types
class FunctionType : public TypeBase {
    Type m_returnType;
    std::vector<Type> m_argumentTypes;
    bool m_isMethod;
    bool m_isNative;
public:
    FunctionType(Type returnType, std::vector<Type> argumentTypes, bool isMethod = false, bool isNative = false);
    ~FunctionType() override = default;

    const Type getReturnType() const;
    const std::vector<Type>& getArgumentTypes() const;

    bool isMethod() const;
    bool isNative() const;

    std::unique_ptr<Typename> toTypename() const override;
};

// Trait types
class TraitType : public TypeBase {
    std::string m_name;
    InsertionOrderMap<std::string, Type> m_methods;
public:
    TraitType(std::string name, InsertionOrderMap<std::string, Type> methods);
    ~TraitType() override = default;

    const std::string& getName() const;

    const InsertionOrderMap<std::string, Type>& getMethods() const;
    std::optional<Type> getMethod(const std::string& name) const;

    std::unique_ptr<Typename> toTypename() const override;
};

// Struct types
class StructType : public TypeBase {
    std::string m_name;
    std::vector<std::shared_ptr<const TraitType>> m_traits;
    InsertionOrderMap<std::string, Type> m_fields;
    InsertionOrderMap<std::string, Type> m_methods;

public:
    StructType(
            std::string name,
            std::vector<std::shared_ptr<const TraitType>> traits,
            InsertionOrderMap<std::string, Type> fields,
            InsertionOrderMap<std::string, Type> methods);
    ~StructType() override = default;

    const std::string& getName() const;

    const std::vector<std::shared_ptr<const TraitType>>& getTraits() const;
    bool hasTrait(const TypeBase& trait) const;
    std::optional<size_t> findTrait(const TypeBase& trait) const;

    std::optional<Type> getProperty(const std::string& name) const;

    const InsertionOrderMap<std::string, Type>& getFields() const;
    std::optional<Type> getField(const std::string& name) const;
    std::optional<size_t> findField(const std::string& name) const;

    const InsertionOrderMap<std::string, Type>& getMethods() const;
    std::optional<Type> getMethod(const std::string& name) const;
    std::optional<size_t> findMethod(const std::string& name) const;

    std::unique_ptr<Typename> toTypename() const override;
};

// Struct constructor types
class ConstructorType : public TypeBase {
    std::shared_ptr<const StructType> m_structType;
    InsertionOrderMap<std::string, Type> m_assocProperties;

public:
    ConstructorType(std::shared_ptr<const StructType> structType, InsertionOrderMap<std::string, Type> assocProperties);
    ~ConstructorType() override = default;

    std::shared_ptr<const StructType> getStructType() const;

    const InsertionOrderMap<std::string, Type>& getAssocProperties() const;
    std::optional<Type> getAssocProperty(const std::string& name) const;
    std::optional<size_t> findAssocProperty(const std::string& name) const;

    std::unique_ptr<Typename> toTypename() const override;
};

#endif //ENACT_TYPE_H
