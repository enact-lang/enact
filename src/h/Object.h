#ifndef ENACT_OBJECT_H
#define ENACT_OBJECT_H

#include <string>
#include "Type.h"
#include "Value.h"

enum class ObjectType {
    STRING,
    ARRAY,
    UPVALUE,
    CLOSURE,
    FUNCTION,
    NATIVE
};

class StringObject;
class ArrayObject;
class UpvalueObject;
class ClosureObject;
class FunctionObject;
class NativeObject;

class Object {
    static Object* m_objects;

    ObjectType m_type;
    Object* m_next = nullptr;

public:
    explicit Object(ObjectType type);
    virtual ~Object() = default;

    static void freeAll();

    template <typename T>
    inline bool is() const;

    template <typename T>
    inline T* as();

    template <typename T>
    inline const T* as() const;

    bool operator==(const Object& object) const;

    virtual std::string toString() const = 0;
    virtual Type getType() const = 0;
};

std::ostream& operator<<(std::ostream& stream, const Object& object);

template<typename T>
inline bool Object::is() const {
    static_assert(IsAny<T, StringObject, ArrayObject, UpvalueObject, ClosureObject, FunctionObject, NativeObject>::value,
                  "Object::is<T>: T must inherit Object.");

    if (std::is_same_v<T, StringObject>) {
        return m_type == ObjectType::STRING;
    } else if (std::is_same_v<T, ArrayObject>) {
        return m_type == ObjectType::ARRAY;
    } else if (std::is_same_v<T, UpvalueObject>) {
        return m_type == ObjectType::UPVALUE;
    } else if (std::is_same_v<T, ClosureObject>) {
        return m_type == ObjectType::CLOSURE;
    } else if (std::is_same_v<T, FunctionObject>) {
        return m_type == ObjectType::FUNCTION;
    } else if (std::is_same_v<T, NativeObject>) {
        return m_type == ObjectType::NATIVE;
    }

    return false;
}

template<typename T>
inline T* Object::as() {
    static_assert(IsAny<T, StringObject, ArrayObject, UpvalueObject, ClosureObject, FunctionObject, NativeObject>::value,
                  "Object::as<T>: T must inherit Object.");

    return static_cast<T*>(this);
}

template<typename T>
inline const T* Object::as() const {
    static_assert(IsAny<T, StringObject, ArrayObject, UpvalueObject, ClosureObject, FunctionObject, NativeObject>::value,
                  "Object::as<T>: T must inherit Object.");
    return static_cast<const T*>(this);
}


class StringObject : public Object {
    std::string m_data;

public:
    explicit StringObject(std::string data);
    ~StringObject() override = default;

    const std::string& asStdString() const;

    std::string toString() const override;
    Type getType() const override;
};

class Value;

class ArrayObject : public Object {
    std::vector<Value> m_vector;

public:
    explicit ArrayObject();
    explicit ArrayObject(std::vector<Value> vector);

    ~ArrayObject() override = default;

    std::optional<Value> at(size_t index) const;

    const std::vector<Value>& asVector() const;

    std::string toString() const override;
    Type getType() const override;
};

class UpvalueObject : public Object {
    uint32_t m_location;

public:
    explicit UpvalueObject(uint32_t location);
    ~UpvalueObject() override = default;

    uint32_t getLocation();

    std::string toString() const override;
    Type getType() const override;
};

class ClosureObject : public Object {
    FunctionObject* m_function;
    std::vector<UpvalueObject*> m_upvalues;

public:
    explicit ClosureObject(FunctionObject* function);
    ~ClosureObject() override = default;

    FunctionObject* getFunction();
    std::vector<UpvalueObject*>& getUpvalues();

    std::string toString() const override;
    Type getType() const override;
};

#include "Chunk.h"

class FunctionObject : public Object {
    Type m_type;
    Chunk m_chunk;
    std::string m_name;
    uint32_t m_upvalueCount = 0;

public:
    explicit FunctionObject(Type type, Chunk chunk, std::string name);
    ~FunctionObject() override = default;

    Chunk& getChunk();
    const std::string& getName() const;
    uint32_t& getUpvalueCount();

    std::string toString() const override;
    Type getType() const override;
};

typedef Value (*NativeFn)(uint8_t argCount, Value* args);

class NativeObject : public Object {
    Type m_type;
    NativeFn m_function;

public:
    explicit NativeObject(Type type, NativeFn function);
    ~NativeObject() override = default;

    NativeFn getFunction();

    std::string toString() const override;
    Type getType() const override;
};

#endif //ENACT_OBJECT_H
