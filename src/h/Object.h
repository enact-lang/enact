#ifndef ENACT_OBJECT_H
#define ENACT_OBJECT_H

#include <string>
#include "Type.h"
#include "Value.h"

enum class ObjectType {
    STRING,
    ARRAY,
    FUNCTION
};

class StringObject;
class ArrayObject;
class FunctionObject;

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

    std::string toString() const;

    virtual Type getType() const = 0;
};

std::ostream& operator<<(std::ostream& stream, const Object& object);

template<typename T>
inline bool Object::is() const {
    static_assert(IsAny<T, StringObject, ArrayObject, FunctionObject>::value,
                  "Object::is<T>: T must be StringObject, ArrayObject, or FunctionObject.");

    if (std::is_same_v<T, StringObject>) {
        return m_type == ObjectType::STRING;
    } else if (std::is_same_v<T, ArrayObject>) {
        return m_type == ObjectType::ARRAY;
    }

    return false;
}

template<typename T>
inline T* Object::as() {
    static_assert(IsAny<T, StringObject, ArrayObject, FunctionObject>::value,
                  "Object::as<T>: T must be StringObject, ArrayObject, or FunctionObject.");

    return static_cast<T*>(this);
}

template<typename T>
inline const T* Object::as() const {
    static_assert(IsAny<T, StringObject, ArrayObject, FunctionObject>::value,
                  "Object::as<T>: T must be StringObject, ArrayObject, or FunctionObject.");
    return static_cast<const T*>(this);
}


class StringObject : public Object {
    std::string m_data;

public:
    explicit StringObject(std::string data);
    ~StringObject() override = default;

    const std::string& asStdString() const;

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

    Type getType() const override;
};

#include "Chunk.h"

class FunctionObject : public Object {
    Type m_type;
    Chunk m_chunk;

public:
    explicit FunctionObject(Type type, Chunk chunk);
    ~FunctionObject() override = default;

    const Chunk& getChunk() const;

    Type getType() const override;
};

#endif //ENACT_OBJECT_H
