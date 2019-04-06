#ifndef ENACT_OBJECT_H
#define ENACT_OBJECT_H

#include <string>

enum class ObjectType {
    STRING
};

class StringObject;

class Object {
    ObjectType m_type;

public:
    explicit Object(ObjectType type);

    template <typename T>
    inline bool is();

    template <typename T>
    inline T* as();
};

template<typename T>
inline bool Object::is() {
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

class StringObject : public Object {
    std::string m_data;

public:

    explicit StringObject(std::string data);
    const std::string& asStdString();
};

#endif //ENACT_OBJECT_H
