#include <sstream>
#include "h/Context.h"
#include "h/VM.h"


VM::VM(Context& context) : m_context{context} {
}

InterpretResult VM::run(FunctionObject *function) {
    try {
        executionLoop(function);
    } catch (const RuntimeError& error) {
        return InterpretResult::RUNTIME_ERROR;
    }

    return InterpretResult::OK;
}

void VM::executionLoop(FunctionObject* function) {
    push(Value{function});

    m_frame = &m_frames[m_frameCount++];
    m_frame->closure = m_context.gc.allocateObject<ClosureObject>(function);
    pop();
    if (m_pc == 0) {
        push(Value{m_frame->closure});
    } else {
        m_stack[0] = Value{m_frame->closure};
    }
    m_frame->ip = function->getChunk().getCode().data() + m_pc;
    m_frame->slotsBegin = 0;

    for (;;) {
        #define NUMERIC_OP(op) \
            do { \
                Value b = pop(); \
                Value a = pop(); \
                if (a.isInt() && b.isInt()) { \
                    push(Value{a.asInt() op b.asInt()}); \
                } else if (a.isDouble() && b.isDouble()) { \
                    push(Value{a.asDouble() op b.asDouble()}); \
                } else if (a.isInt() && b.isDouble()) { \
                    push(Value{a.asInt() op b.asDouble()}); \
                } else { \
                    push(Value{a.asDouble() op b.asInt()}); \
                } \
            } while (false)

        if (m_context.options.flagEnabled(Flag::DEBUG_TRACE_EXECUTION)) {
            std::cout << "    ";
            for (Value value : m_stack) {
                std::cout << "[ " << value << " ] ";
            }
            std::cout << "\n";

            std::cout << m_frame->closure->getFunction()->getChunk()
                    .disassembleInstruction(
                            m_frame->ip - m_frame->closure->getFunction()->getChunk().getCode().data()).first;
        }

        Value* slots = &m_stack[m_frame->slotsBegin];

        switch (static_cast<OpCode>(readByte())) {
            case OpCode::CONSTANT: {
                Value constant = readConstant();
                push(constant);
                break;
            }

            case OpCode::CONSTANT_LONG: {
                Value constant = readConstantLong();
                push(constant);
                break;
            }

            case OpCode::TRUE: push(Value{true}); break;
            case OpCode::FALSE: push(Value{false}); break;
            case OpCode::NIL: push(Value{}); break;

            case OpCode::CHECK_INT: {
                Value value = peek(0);
                if (!value.getType()->isInt()) {
                    throw runtimeError("Expected a value of type 'int', but got a value of type '"
                                 + value.getType()->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_NUMERIC: {
                Value value = peek(0);
                if (!value.getType()->isNumeric()) {
                    throw runtimeError("Expected a value of type 'int' or 'float', but got a value of type '"
                            + value.getType()->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_BOOL: {
                Value value = peek(0);
                if (!value.getType()->isBool()) {
                    throw runtimeError("Expected a value of type 'bool', but got a value of type '"
                            + value.getType()->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_REFERENCE: {
                Value value = peek(0);
                if (value.getType()->isPrimitive()) {
                    throw runtimeError("Only reference types can be copied, not a value of type '"
                                 + value.getType()->toString() + "'.");
                }
                break;
            }
            case OpCode::CHECK_INDEXABLE: {
                Value array = peek(0);
                if (!array.getType()->isArray()) {
                    throw runtimeError("Expected an array, but got a value of type '" + array.getType()->toString() +
                            "' instead.");
                }
                break;
            }
            case OpCode::CHECK_ALLOTABLE: {
                Type shouldBe = peek(0).getType()->as<ArrayType>()->getElementType();
                Type valueType = peek(1).getType();

                if (!valueType->looselyEquals(*shouldBe)) {
                    throw runtimeError("Expected a value of type '" + shouldBe->toString() +
                        "' to assign in array, but got a value of type '" + valueType->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_TYPE: {
                Type shouldBe = readConstant().asObject()->getType();
                Value value = peek(0);
                if (!shouldBe->looselyEquals(*value.getType())) {
                    throw runtimeError("Expected a value of type '" + shouldBe->toString() +
                            "' but got a value of type '" + value.getType()->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_TYPE_LONG: {
                Type shouldBe = readConstantLong().asObject()->getType();
                Value value = peek(0);
                if (!shouldBe->looselyEquals(*value.getType())) {
                    throw runtimeError("Expected a value of type '" + shouldBe->toString() +
                                 "' but got a value of type '" + value.getType()->toString() + "' instead.");
                }
                break;
            }

            case OpCode::NEGATE: {
                Value value = pop();
                if (value.isInt()) {
                    push(Value{-value.asInt()});
                } else {
                    push(Value{-value.asDouble()});
                }
                break;
            }
            case OpCode::NOT: push(Value{!pop().asBool()}); break;

            case OpCode::COPY: push(Value{pop().asObject()->clone()}); break;

            case OpCode::ADD: NUMERIC_OP(+); break;
            case OpCode::SUBTRACT: NUMERIC_OP(-); break;
            case OpCode::MULTIPLY: NUMERIC_OP(*); break;
            case OpCode::DIVIDE: NUMERIC_OP(/); break;

            case OpCode::LESS: NUMERIC_OP(<); break;
            case OpCode::GREATER: NUMERIC_OP(>); break;
            case OpCode::EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value{a == b});
                break;
            }


            case OpCode::ARRAY: {
                uint8_t length = readByte();
                Type type = readConstant().asObject()->as<TypeObject>()->getContainedType();
                auto* array = m_context.gc.allocateObject<ArrayObject>(length, type);
                if (length != 0) {
                    for (uint8_t i = length; i-- > 0;) {
                        array->at(i) = pop();
                    }
                }
                push(Value{array});
                break;
            }
            case OpCode::ARRAY_LONG: {
                uint32_t length = readLong();
                Type type = readConstantLong().asObject()->as<TypeObject>()->getContainedType();
                auto* array = m_context.gc.allocateObject<ArrayObject>(length, type);
                if (length != 0) {
                    for (uint32_t i = length; i-- > 0;) {
                        array->at(i) = pop();
                    }
                }
                push(Value{array});
                break;
            }

            case OpCode::GET_ARRAY_INDEX: {
                int index = pop().asInt();
                ArrayObject* array = pop().asObject()->as<ArrayObject>();

                if (index >= array->length()) {
                    throw runtimeError("Array index '" + std::to_string(index) + "' is out of bounds for array of "
                             + "length '" + std::to_string(array->asVector().size()) + "'.");
                }

                push(array->at(index));
                break;
            }
            case OpCode::SET_ARRAY_INDEX: {
                int index = pop().asInt();
                ArrayObject* array = pop().asObject()->as<ArrayObject>();
                Value newValue = peek(0);

                if (index >= array->length()) {
                    throw runtimeError("Array index '" + std::to_string(index) + "' is out of bounds for array of "
                                 + "length '" + std::to_string(array->asVector().size()) + "'.");
                }

                array->at(index) = newValue;
                break;
            }

            case OpCode::POP: pop(); break;

            case OpCode::GET_LOCAL: push(slots[readByte()]); break;
            case OpCode::GET_LOCAL_LONG: push(slots[readLong()]); break;

            case OpCode::SET_LOCAL: slots[readByte()] = peek(0); break;
            case OpCode::SET_LOCAL_LONG: slots[readLong()] = peek(0); break;

            case OpCode::GET_UPVALUE: {
                uint8_t slot = readByte();
                UpvalueObject* upvalue = m_frame->closure->getUpvalues()[slot];
                push(upvalue->isClosed() ?
                        upvalue->getClosed() :
                        m_stack[upvalue->getLocation()]);
                break;
            }
            case OpCode::GET_UPVALUE_LONG: {
                uint8_t slot = readByte();
                UpvalueObject* upvalue = m_frame->closure->getUpvalues()[slot];
                push(upvalue->isClosed() ?
                      upvalue->getClosed() :
                      m_stack[upvalue->getLocation()]);
                break;
            }

            case OpCode::SET_UPVALUE: {
                uint8_t slot = readByte();
                m_stack[m_frame->closure->getUpvalues()[slot]->getLocation()] = peek(0);
                break;
            }
            case OpCode::SET_UPVALUE_LONG: {
                uint32_t slot = readLong();
                m_stack[m_frame->closure->getUpvalues()[slot]->getLocation()] = peek(0);
                break;
            }

            case OpCode::GET_FIELD: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint8_t index = readByte();
                push(instance->field(index));

                break;
            }
            case OpCode::GET_FIELD_LONG: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = readLong();
                push(instance->field(index));

                break;
            }
            case OpCode::SET_FIELD: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint8_t index = readByte();
                instance->field(index) = peek(0);

                break;
            }
            case OpCode::SET_FIELD_LONG: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = readLong();
                instance->field(index) = peek(0);

                break;
            }

            case OpCode::GET_METHOD: {
                auto* instance = peek(0)
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = readByte();
                ClosureObject* method = instance
                        ->getStruct()
                        ->method(index);

                auto* bound = m_context.gc.allocateObject<BoundMethodObject>(Value{instance}, method);
                pop(); // Pop the instance
                push(Value{bound});
                break;
            }
            case OpCode::GET_METHOD_LONG: {
                auto* instance = peek(0)
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = readLong();
                ClosureObject* method = instance
                        ->getStruct()
                        ->method(index);

                auto* bound = m_context.gc.allocateObject<BoundMethodObject>(Value{instance}, method);
                pop(); // Pop the instance;
                push(Value{bound});
                break;
            }

            case OpCode::GET_ASSOC: {
                auto* struct_ = peek(0)
                        .asObject()
                        ->as<StructObject>();

                uint32_t index = readByte();
                Value assoc = struct_->assoc(index);

                pop();
                push(assoc);
                break;
            }
            case OpCode::GET_ASSOC_LONG: {
                auto* struct_ = peek(0)
                        .asObject()
                        ->as<StructObject>();

                uint32_t index = readLong();
                Value assoc = struct_->assoc(index);

                pop();
                push(assoc);
                break;
            }

            case OpCode::GET_PROPERTY_DYNAMIC: {
                Value maybeObject = peek(0);
                if (!maybeObject.isObject()) {
                    throw runtimeError("Only instances and constructors have properties, not a value of type '" +
                                       maybeObject.getType()->toString() + "'.");
                }

                Object* object = maybeObject.asObject();
                const std::string& name = readConstant()
                        .asObject()
                        ->as<StringObject>()
                        ->asStdString();

                if (object->is<InstanceObject>()) {
                    auto* instance = peek(0)
                            .asObject()
                            ->as<InstanceObject>();

                    Value property;

                    if (auto field = instance->fieldNamed(name)) {
                        property = *field;
                    } else if (auto method = instance->getStruct()->methodNamed(name)) {
                        auto* bound = m_context.gc.allocateObject<BoundMethodObject>(Value{instance}, *method);
                        property = Value{bound};
                    } else {
                        throw runtimeError("Instance of type '" + instance->getType()->toString() +
                                           "' does not have a property named '" + name + "'.");
                    }

                    pop(); // Pop the instance
                    push(property);
                } else if (object->is<StructObject>()) {
                    auto* struct_ = pop()
                            .asObject()
                            ->as<StructObject>();

                    std::optional<std::reference_wrapper<Value>> assoc;
                    if (!(assoc = struct_->assocNamed(name))) {
                        throw runtimeError("Struct '" + assoc->get().getType()->toString() +
                                           "' does not have an associated function named '" + name + "'.");
                    }

                    push(*assoc);
                } else {
                    throw runtimeError("Only instances and constructors have properties, not a value of type '" +
                                       object->getType()->toString() + "'.");
                }

                break;
            }
            case OpCode::GET_PROPERTY_DYNAMIC_LONG: {
                Value maybeObject = peek(0);
                if (!maybeObject.isObject()) {
                    throw runtimeError("Only instances and constructors have properties, not a value of type '" +
                                       maybeObject.getType()->toString() + "'.");
                }

                Object* object = maybeObject.asObject();
                const std::string& name = readConstantLong()
                        .asObject()
                        ->as<StringObject>()
                        ->asStdString();

                if (object->is<InstanceObject>()) {
                    auto* instance = peek(0)
                            .asObject()
                            ->as<InstanceObject>();

                    Value property;

                    if (auto field = instance->fieldNamed(name)) {
                        property = *field;
                    } else if (auto method = instance->getStruct()->methodNamed(name)) {
                        auto* bound = m_context.gc.allocateObject<BoundMethodObject>(Value{instance}, *method);
                        property = Value{bound};
                    } else {
                        throw runtimeError("Instance of type '" + instance->getType()->toString() +
                                           "' does not have a property named '" + name + "'.");
                    }

                    pop(); // Pop the instance
                    push(property);
                } else if (object->is<StructObject>()) {
                    auto* struct_ = pop()
                            .asObject()
                            ->as<StructObject>();

                    std::optional<std::reference_wrapper<Value>> assoc;
                    if (!(assoc = struct_->assocNamed(name))) {
                        throw runtimeError("Struct '" + assoc->get().getType()->toString() +
                                           "' does not have an associated function named '" + name + "'.");
                    }

                    push(*assoc);
                } else {
                    throw runtimeError("Only instances and constructors have properties, not a value of type '" +
                                       object->getType()->toString() + "'.");
                }

                break;
            }

            case OpCode::SET_PROPERTY_DYNAMIC_LONG: {
                Value maybeInstance = peek(0);
                if (!maybeInstance.isObject() && !maybeInstance.asObject()->is<InstanceObject>()) {
                    throw runtimeError("Only instances have assignable fields, not a value of type '" +
                                       maybeInstance.getType()->toString() + "'.");
                }

                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                const std::string& name = readConstantLong()
                        .asObject()
                        ->as<StringObject>()
                        ->asStdString();

                std::optional<std::reference_wrapper<Value>> maybeField;
                if (!(maybeField = instance->fieldNamed(name))) {
                    throw runtimeError("Instance of type '" + instance->getType()->toString() +
                                       "' does not have a field named '" + name + "'.");
                }

                Value& field = maybeField.value().get();
                Value value = peek(0);

                if (!field.getType()->looselyEquals(*value.getType())) {
                    throw runtimeError("Cannot assign a value of type '" + value.getType()->toString() +
                                       "' to field '" + name + "' of type '" + field.getType()->toString() + "'.");
                }

                field = peek(0);
                break;
            }

            case OpCode::JUMP: {
                uint16_t jumpSize = readShort();
                m_frame->ip += jumpSize;
                break;
            }
            case OpCode::JUMP_IF_TRUE: {
                uint16_t jumpSize = readShort();
                if (peek(0).asBool()) {
                    m_frame->ip += jumpSize;
                }
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t jumpSize = readShort();
                if (!peek(0).asBool()) {
                    m_frame->ip += jumpSize;
                }
                break;
            }

            case OpCode::LOOP: {
                uint16_t jumpSize = readShort();
                m_frame->ip -= jumpSize;
                break;
            }

            case OpCode::CALL_FUNCTION: {
                uint8_t argCount = readByte();
                auto* closure = peek(argCount)
                        .asObject()
                        ->as<ClosureObject>();

                callFunction(closure, argCount);
                m_frame = &m_frames[m_frameCount - 1];
                break;
            }

            case OpCode::CALL_BOUND_METHOD: {
                uint8_t argCount = readByte();
                auto* bound = peek(argCount)
                        .asObject()
                        ->as<BoundMethodObject>();

                callFunction(bound->method(), argCount);
                m_frame = &m_frames[m_frameCount - 1];
                push(bound->receiver());

                break;
            }

            case OpCode::CALL_CONSTRUCTOR: {
                uint8_t argCount = readByte();
                auto* struct_ = peek(argCount)
                        .asObject()
                        ->as<StructObject>();

                callConstructor(struct_, argCount);
                break;
            }

            case OpCode::CALL_NATIVE: {
                uint8_t argCount = readByte();
                auto* native = peek(argCount)
                        .asObject()
                        ->as<NativeObject>();

                callNative(native, argCount);
                break;
            }

            case OpCode::CALL_DYNAMIC: {
                uint8_t argCount = readByte();
                Value calleeValue = peek(argCount);

                if (!calleeValue.isObject()) {
                    throw runtimeError("Only functions and constructors can be called, not a value of type '"
                                 + calleeValue.getType()->toString() + ".");
                }

                Object* callee = calleeValue.asObject();

                if (callee->is<ClosureObject>()) {
                    checkFunctionCallable(callee->getType()->as<FunctionType>(), argCount);
                    callFunction(callee->as<ClosureObject>(), argCount);

                    // TODO: incorporate this in callFunction()
                    m_frame = &m_frames[m_frameCount - 1];
                } else if (callee->is<BoundMethodObject>()) {
                    auto* bound = callee->as<BoundMethodObject>();
                    checkFunctionCallable(bound->method()->getType()->as<FunctionType>(), argCount);

                    callFunction(bound->method(), argCount);
                    m_frame = &m_frames[m_frameCount - 1];
                    push(bound->receiver());
                } else if (callee->is<StructObject>()) {
                    checkConstructorCallable(callee->getType()->as<ConstructorType>(), argCount);
                    callConstructor(callee->as<StructObject>(), argCount);
                } else if (callee->is<NativeObject>()) {
                    // Native functions are of a regular FunctionType
                    checkFunctionCallable(callee->getType()->as<FunctionType>(), argCount);
                    callNative(callee->as<NativeObject>(), argCount);
                } else {
                    throw runtimeError("Only functions and constructors can be called, not a value of type '"
                                 + callee->getType()->toString() + ".");
                }
                break;
            }

            case OpCode::CLOSURE: {
                FunctionObject* function = readConstant().asObject()->as<FunctionObject>();
                encloseFunction(function);
                break;
            }

            case OpCode::CLOSURE_LONG: {
                FunctionObject* function = readConstantLong().asObject()->as<FunctionObject>();
                encloseFunction(function);
                break;
            }

            case OpCode::CLOSE_UPVALUE:
                closeUpvalues(m_stack.size() - 1);
                pop();
                break;

            case OpCode::RETURN: {
                Value result = pop();

                closeUpvalues(m_frame->slotsBegin);

                m_frameCount--;
                if (m_frameCount == 0) {
                    pop();
                    return;
                }

                m_stack.erase(m_stack.begin() + m_frame->slotsBegin, m_stack.end());
                push(result);

                m_frame = &m_frames[m_frameCount - 1];
                break;
            }

            case OpCode::STRUCT: {
                auto type = std::static_pointer_cast<const ConstructorType>(
                        readConstant()
                            .asObject()
                            ->as<TypeObject>()
                            ->getContainedType());

                makeConstructor(type);
                break;
            }
            case OpCode::STRUCT_LONG: {
                auto type = std::static_pointer_cast<const ConstructorType>(
                        readConstant()
                            .asObject()
                            ->as<TypeObject>()
                            ->getContainedType());

                makeConstructor(type);
                break;
            }

            case OpCode::PAUSE: {
                m_pc = m_frame->ip - function->getChunk().getCode().data();
                m_frameCount--;
                return;
            }
        }
    }
}

inline void VM::callFunction(ClosureObject* closure, uint8_t argCount) {
    if (m_frameCount == FRAMES_MAX) {
        throw runtimeError("Stack overflow.");
    }

    CallFrame* frame = &m_frames[m_frameCount++];
    frame->closure = closure;
    frame->ip = closure->getFunction()->getChunk().getCode().data();

    frame->slotsBegin = m_stack.size() - argCount - 1;
}

inline void VM::callConstructor(StructObject *struct_, uint8_t argCount) {
    auto* instance = m_context.gc.allocateObject<InstanceObject>(
            struct_,
            std::move(std::vector<Value>{m_stack.end() - argCount, m_stack.end()}));

    m_stack.erase(m_stack.end() - argCount - 1, m_stack.end());

    push(Value{instance});
}

inline void VM::callNative(NativeObject *native, uint8_t argCount) {
    NativeFn fn = native->getFunction();

    Value result = fn(argCount, &m_stack.back() - argCount + 1);
    m_stack.erase(m_stack.end() - argCount - 1, m_stack.end());

    push(result);
}

inline void VM::checkFunctionCallable(const FunctionType *type, uint8_t argCount) {
    const std::vector<Type>& paramTypes = type->getArgumentTypes();

    if (argCount != paramTypes.size()) {
        std::stringstream s;
        s << "Expected " << paramTypes.size() << " arguments to function, but got " <<
          static_cast<size_t>(argCount) << ".";
        throw runtimeError(s.str());
    }

    for (uint8_t i = argCount; i-- > 0;) {
        Type shouldBe = paramTypes[paramTypes.size() - 1 - i];
        Type argumentType = peek(i).getType();

        if (!argumentType->looselyEquals(*shouldBe)) {
            std::stringstream s;
            s << "Expected argument " << paramTypes.size() - static_cast<size_t>(i) << " to be of type '" <<
              shouldBe->toString() << "' but got value of type '" + argumentType->toString() <<
              "' instead.";
            throw runtimeError(s.str());
        }
    }
}

inline void VM::checkConstructorCallable(const ConstructorType *type, uint8_t argCount) {
    std::vector<std::reference_wrapper<const Type>> paramTypes = type
            ->getStructType()
            ->getFields()
            .values();

    if (argCount != paramTypes.size()) {
        std::stringstream s;
        s << "Expected " << paramTypes.size() << " arguments to constructor, but got " <<
          static_cast<size_t>(argCount) << ".";
        throw runtimeError(s.str());
    }

    for (uint8_t i = argCount; i-- > 0;) {
        Type shouldBe = paramTypes[paramTypes.size() - 1 - i];
        Type argumentType = peek(i).getType();

        if (!argumentType->looselyEquals(*shouldBe)) {
            std::stringstream s;
            s << "Expected argument " << paramTypes.size() - static_cast<size_t>(i) << " to be of type '" <<
              shouldBe->toString() << "' but got value of type '" + argumentType->toString() <<
              "' instead.";
            throw runtimeError(s.str());
        }
    }
}

inline void VM::encloseFunction(FunctionObject *function) {
    push(Value{function});
    auto* closure = m_context.gc.allocateObject<ClosureObject>(function);
    pop();
    push(Value{closure});

    for (size_t i = 0; i < closure->getUpvalues().size(); ++i) {
        uint8_t isLocal = readByte();
        uint32_t index;
        if (i < UINT8_MAX) {
            index = readByte();
        } else {
            index = readLong();
        }

        if (isLocal) {
            closure->getUpvalues()[i] = captureUpvalue(m_frame->slotsBegin + index);
        } else {
            closure->getUpvalues()[i] = m_frame->closure->getUpvalues()[i];
        }
    }
}

inline void VM::makeConstructor(std::shared_ptr<const ConstructorType> type) {
    // Collect methods
    size_t methodsBeginIndex = m_stack.size();
    size_t methodCount = type->getStructType()->getMethods().length();
    for (uint32_t i = 0; i < methodCount; ++i) {
        auto* function = readConstant().asObject()->as<FunctionObject>();
        encloseFunction(function);
    }

    // Collect assocs
    size_t assocsBeginIndex = m_stack.size();
    size_t assocCount = type->getAssocProperties().length();
    for (uint32_t i = 0; i < assocCount; ++i) {
        auto* function = readConstant().asObject()->as<FunctionObject>();
        encloseFunction(function);
    }

    // Move methods off the stack
    auto methodsBegin = m_stack.begin() + methodsBeginIndex;
    auto methodsEnd = m_stack.begin() + methodsBeginIndex + methodCount;
    std::vector<ClosureObject*> methods{};
    for (auto it = methodsBegin; it != methodsEnd; ++it) {
        methods.push_back(it->asObject()->as<ClosureObject>());
    }

    // Move assocs off the stack
    auto assocsBegin = m_stack.begin() + assocsBeginIndex;
    auto assocsEnd = m_stack.begin() + assocsBeginIndex + assocCount;
    std::vector<Value> assocs{assocsBegin, assocsEnd};

    auto* struct_ = m_context.gc.allocateObject<StructObject>(type, std::move(methods), std::move(assocs));
    push(Value{struct_});

    // Erase the moved elements
    m_stack.erase(methodsBegin, assocsEnd);
}

inline uint8_t VM::readByte() {
    return *m_frame->ip++;
}

inline uint16_t VM::readShort() {
    return static_cast<uint16_t>(readByte() | (readByte() << 8u));
}

inline uint32_t VM::readLong() {
    return static_cast<uint32_t>(readByte() | (readByte() << 8u) | (readByte() << 16u));
}

inline Value VM::readConstant() {
    return m_frame
            ->closure
            ->getFunction()
            ->getChunk()
            .getConstants()
            [readByte()];
}

inline Value VM::readConstantLong() {
    return m_frame
            ->closure
            ->getFunction()
            ->getChunk()
            .getConstants()
    [readLong()];
}

void VM::push(Value value) {
    m_stack.push_back(value);
}

Value VM::pop() {
    ENACT_ASSERT(!m_stack.empty(), "Stack underflow!");
    Value value = m_stack.back();
    m_stack.pop_back();
    return value;
}

Value VM::peek(size_t depth) {
    return m_stack[m_stack.size() - 1 - depth];
}

UpvalueObject* VM::captureUpvalue(uint32_t location) {
    UpvalueObject* prevUpvalue = nullptr;
    UpvalueObject* upvalue = m_openUpvalues;

    while (upvalue != nullptr && upvalue->getLocation() == location) {
        prevUpvalue = upvalue;
        upvalue = upvalue->getNext();
    }

    if (upvalue != nullptr && upvalue->getLocation() == location) return upvalue;

    auto* createdUpvalue = m_context.gc.allocateObject<UpvalueObject>(location);
    createdUpvalue->setNext(upvalue);

    if (prevUpvalue == nullptr) {
        m_openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->setNext(createdUpvalue);
    }

    return createdUpvalue;
}

void VM::closeUpvalues(uint32_t last) {
    while (m_openUpvalues != nullptr && m_openUpvalues->getLocation() >= last) {
        UpvalueObject* upvalue = m_openUpvalues;
        upvalue->setClosed(m_stack[upvalue->getLocation()]);
        m_openUpvalues = upvalue->getNext();
    }
}

VM::RuntimeError VM::runtimeError(const std::string& msg) {
    CallFrame* frame = &m_frames[m_frameCount - 1];
    size_t instruction = frame->ip - frame->closure->getFunction()->getChunk().getCode().data();
    line_t line = frame->closure->getFunction()->getChunk().getLine(instruction);

    const std::string source = m_context.getSourceLine(line);

    std::cerr << "[line " << line << "] Error here:\n    " << source << "\n    ";
    for (int i = 0; i < source.size(); ++i) {
        std::cerr << "^";
    }
    std::cerr << "\n" << msg << "\n";
    for (int i = m_frameCount - 1; i >= 0; --i) {
        CallFrame* frame = &m_frames[i];
        FunctionObject* function = frame->closure->getFunction();

        // -1 because the IP is sitting on the next instruction to be executed
        size_t instruction = frame->ip - function->getChunk().getCode().data() - 1;
        std::cerr << "[line " << function->getChunk().getLine(instruction) << "] in ";
        if (function->getName().empty()) {
            std::cerr << "script\n";
        } else {
            std::cerr << function->getName() << "()\n";
        }
    }

    return RuntimeError{};
}