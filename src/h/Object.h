#ifndef ENACT_OBJECT_H
#define ENACT_OBJECT_H

#include <string>

enum class ObjectType {
    STRING
};

class StringObject;

class Object {
    static Object* m_objects;

    ObjectType m_type;
    Object* m_next = nullptr;

public:
    explicit Object(ObjectType type);

    template <typename T>
    inline bool is() const;

    template <typename T>
    inline T* as();

    template <typename T>
    inline const T* as() const;

    std::string toString() const;
};

std::ostream& operator<<(std::ostream& stream, const Object& object);

template<typename T>
inline bool Object::is() const {
    static_assert(std::is_same_v<T, StringObject>, "Object can only be a StringObject.");

    if (std::is_same_v<T, StringObject>) {
        return m_type == ObjectType::STRING;
    }

    return false;
}

template<typename T>
inline T* Object::as() {
    static_assert(std::is_same_v<T, StringObject>, "Object can only be a StringObject.");
    return static_cast<T*>(this);
}

template<typename T>
inline const T* Object::as() const {
    static_assert(std::is_same_v<T, StringObject>, "Object can only be a StringObject.");
    return static_cast<const T*>(this);
}


class StringObject : public Object {
    std::string m_data;

public:

    explicit StringObject(std::string data);
    const std::string& asStdString() const;
};

#endif //ENACT_OBJECT_H
