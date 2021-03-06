#ifndef ENACT_OBJECT_H
#define ENACT_OBJECT_H

#include <string>

#include "../bytecode/Chunk.h"
#include "../type/Type.h"

#include "Value.h"

namespace enact {
    enum class ObjectType {
        STRING,
        ARRAY,
        UPVALUE,
        CLOSURE,
        STRUCT,
        INSTANCE,
        BOUND_METHOD,
        FUNCTION,
        NATIVE,
        TYPE
    };

    class StringObject;

    class ArrayObject;

    class UpvalueObject;

    class ClosureObject;

    class StructObject;

    class InstanceObject;

    class BoundMethodObject;

    class FunctionObject;

    class NativeObject;

    class TypeObject;

    class VM;

    class Object {
        friend class GC;

        ObjectType m_type;
        bool m_isMarked{false};

    public:
        explicit Object(ObjectType type);

        virtual ~Object();

        template<typename T>
        inline bool is() const;

        template<typename T>
        inline T *as();

        template<typename T>
        inline const T *as() const;

        bool operator==(const Object &object) const;

        virtual void mark();

        virtual void unmark();

        virtual bool isMarked();

        virtual std::string toString() const = 0;

        virtual Type getType() const = 0;

        virtual Object *clone() const = 0;

        virtual size_t size() const = 0;
    };

    std::ostream &operator<<(std::ostream &stream, const Object &object);

    template<typename T>
    inline bool Object::is() const {
        static_assert(std::is_base_of_v<Object, T>,
                      "Object::is<T>: T must derive from Object.");

        if (std::is_same_v<T, StringObject>) {
            return m_type == ObjectType::STRING;
        } else if (std::is_same_v<T, ArrayObject>) {
            return m_type == ObjectType::ARRAY;
        } else if (std::is_same_v<T, UpvalueObject>) {
            return m_type == ObjectType::UPVALUE;
        } else if (std::is_same_v<T, ClosureObject>) {
            return m_type == ObjectType::CLOSURE;
        } else if (std::is_same_v<T, StructObject>) {
            return m_type == ObjectType::STRUCT;
        } else if (std::is_same_v<T, InstanceObject>) {
            return m_type == ObjectType::INSTANCE;
        } else if (std::is_same_v<T, BoundMethodObject>) {
            return m_type == ObjectType::BOUND_METHOD;
        } else if (std::is_same_v<T, FunctionObject>) {
            return m_type == ObjectType::FUNCTION;
        } else if (std::is_same_v<T, NativeObject>) {
            return m_type == ObjectType::NATIVE;
        } else if (std::is_same_v<T, TypeObject>) {
            return m_type == ObjectType::TYPE;
        }

        return false;
    }

    template<typename T>
    inline T *Object::as() {
        static_assert(std::is_base_of_v<Object, T>,
                      "Object::as<T>: T must derive from Object.");

        return static_cast<T *>(this);
    }

    template<typename T>
    inline const T *Object::as() const {
        static_assert(std::is_base_of_v<Object, T>,
                      "Object::as<T>: T must derive from Object.");
        return static_cast<const T *>(this);
    }


    class StringObject : public Object {
        std::string m_data;

    public:
        explicit StringObject(std::string data);

        ~StringObject() override = default;

        const std::string &asStdString() const;

        std::string toString() const override;

        Type getType() const override;

        StringObject *clone() const override;

        size_t size() const override;
    };

    class ArrayObject : public Object {
        std::vector<Value> m_vector;
        Type m_type;

    public:
        explicit ArrayObject(Type type);

        explicit ArrayObject(size_t length, Type type);

        explicit ArrayObject(std::vector<Value> vector, Type type);

        ~ArrayObject() override = default;

        size_t length() const;

        Value &at(size_t index);

        const Value &at(size_t index) const;

        void append(Value value);

        const std::vector<Value> &asVector() const;

        std::string toString() const override;

        Type getType() const override;

        ArrayObject *clone() const override;

        size_t size() const override;
    };

    class UpvalueObject : public Object {
        uint32_t m_location;
        UpvalueObject *m_next = nullptr;

        bool m_isClosed = false;
        Value m_closed{};

    public:
        explicit UpvalueObject(uint32_t location);

        ~UpvalueObject() override = default;

        uint32_t getLocation();

        UpvalueObject *getNext();

        void setNext(UpvalueObject *next);

        bool isClosed() const;

        Value getClosed() const;

        void setClosed(Value value);

        std::string toString() const override;

        Type getType() const override;

        UpvalueObject *clone() const override;

        size_t size() const override;
    };

    class ClosureObject : public Object {
        FunctionObject *m_function{nullptr};
        std::vector<UpvalueObject *> m_upvalues{};

    public:
        explicit ClosureObject(FunctionObject *function);

        ~ClosureObject() override = default;

        FunctionObject *getFunction();

        std::vector<UpvalueObject *> &getUpvalues();

        std::string toString() const override;

        Type getType() const override;

        ClosureObject *clone() const override;

        size_t size() const override;
    };

    class StructObject : public Object {
        std::shared_ptr<const ConstructorType> m_constructorType;
        std::vector<ClosureObject *> m_methods;
        std::vector<Value> m_assocs;

    public:
        StructObject(std::shared_ptr<const ConstructorType> constructorType, std::vector<ClosureObject *> methods,
                     std::vector<Value> assocs);

        ~StructObject() override = default;

        const std::string &getName() const;

        std::vector<ClosureObject *> &methods();

        ClosureObject *method(uint32_t index);

        std::optional<ClosureObject *> methodNamed(const std::string &name);


        std::vector<Value> &assocs();

        Value &assoc(uint32_t index);

        std::optional<std::reference_wrapper<Value>> assocNamed(const std::string &name);

        std::string toString() const override;

        Type getType() const override;

        StructObject *clone() const override;

        size_t size() const override;
    };

    class InstanceObject : public Object {
        StructObject *m_struct;
        std::vector<Value> m_fields;

    public:
        InstanceObject(StructObject *struct_, std::vector<Value> fields);

        ~InstanceObject() override = default;

        StructObject *getStruct();

        std::vector<Value> &fields();

        Value &field(uint32_t index);

        std::optional<std::reference_wrapper<Value>> fieldNamed(const std::string &name);

        std::string toString() const override;

        Type getType() const override;

        StructObject *clone() const override;

        size_t size() const override;
    };

    class BoundMethodObject : public Object {
        Value m_receiver;
        ClosureObject *m_method;

    public:
        BoundMethodObject(Value receiver, ClosureObject *method);

        ~BoundMethodObject() override = default;

        Value receiver();

        ClosureObject *method();

        std::string toString() const override;

        Type getType() const override;

        BoundMethodObject *clone() const override;

        size_t size() const override;
    };

    class FunctionObject : public Object {
        Type m_type{nullptr};
        Chunk m_chunk{};
        std::string m_name{};
        uint32_t m_upvalueCount = 0;

    public:
        explicit FunctionObject(Type type, Chunk chunk, std::string name);

        ~FunctionObject() override = default;

        Chunk &getChunk();

        const std::string &getName() const;

        uint32_t &getUpvalueCount();

        std::string toString() const override;

        Type getType() const override;

        FunctionObject *clone() const override;

        size_t size() const override;
    };

    typedef Value (*NativeFn)(uint8_t argCount, Value *args);

    class NativeObject : public Object {
        Type m_type{nullptr};
        NativeFn m_function{nullptr};

    public:
        explicit NativeObject(Type type, NativeFn function);

        ~NativeObject() override = default;

        NativeFn getFunction();

        std::string toString() const override;

        Type getType() const override;

        NativeObject *clone() const override;

        size_t size() const override;
    };

    class TypeObject : public Object {
        Type m_containedType{nullptr};

    public:
        explicit TypeObject(Type containedType);

        ~TypeObject() override = default;

        Type getContainedType();

        std::string toString() const override;

        Type getType() const override;

        TypeObject *clone() const override;

        size_t size() const override;
    };
}

#endif //ENACT_OBJECT_H
