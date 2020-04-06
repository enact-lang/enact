#include <sstream>
#include "h/Context.h"

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
    CallFrame* frame;

    push(Value{function});

    frame = &m_frames[m_frameCount++];
    frame->closure = m_context.gc.allocateObject<ClosureObject>(function);
    pop();
    if (m_pc == 0) {
        push(Value{frame->closure});
    } else {
        m_stack[0] = Value{frame->closure};
    }
    frame->ip = function->getChunk().getCode().data() + m_pc;
    frame->slotsBegin = 0;

    for (;;) {
        #define READ_BYTE() (*frame->ip++)
        #define READ_SHORT() (static_cast<uint16_t>(READ_BYTE() | (READ_BYTE() << 8)))
        #define READ_LONG() (static_cast<uint32_t>(READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16)))
        #define READ_CONSTANT() ((frame->closure->getFunction()->getChunk().getConstants())[READ_BYTE()])
        #define READ_CONSTANT_LONG() ((frame->closure->getFunction()->getChunk().getConstants())[READ_LONG()])
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

            std::cout << frame->closure->getFunction()->getChunk()
                    .disassembleInstruction(
                            frame->ip - frame->closure->getFunction()->getChunk().getCode().data()).first;
        }

        Value* slots = &m_stack[frame->slotsBegin];

        switch (static_cast<OpCode>(READ_BYTE())) {
            case OpCode::CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OpCode::CONSTANT_LONG: {
                Value constant = READ_CONSTANT_LONG();
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
                Type shouldBe = READ_CONSTANT().asObject()->getType();
                Value value = peek(0);
                if (!shouldBe->looselyEquals(*value.getType())) {
                    throw runtimeError("Expected a value of type '" + shouldBe->toString() +
                            "' but got a value of type '" + value.getType()->toString() + "' instead.");
                }
                break;
            }
            case OpCode::CHECK_TYPE_LONG: {
                Type shouldBe = READ_CONSTANT_LONG().asObject()->getType();
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
                uint8_t length = READ_BYTE();
                Type type = READ_CONSTANT().asObject()->as<TypeObject>()->getContainedType();
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
                uint32_t length = READ_LONG();
                Type type = READ_CONSTANT_LONG().asObject()->as<TypeObject>()->getContainedType();
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

            case OpCode::GET_LOCAL: push(slots[READ_BYTE()]); break;
            case OpCode::GET_LOCAL_LONG: push(slots[READ_LONG()]); break;

            case OpCode::SET_LOCAL: slots[READ_BYTE()] = peek(0); break;
            case OpCode::SET_LOCAL_LONG: slots[READ_LONG()] = peek(0); break;

            case OpCode::GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                UpvalueObject* upvalue = frame->closure->getUpvalues()[slot];
                push(upvalue->isClosed() ?
                        upvalue->getClosed() :
                        m_stack[upvalue->getLocation()]);
                break;
            }
            case OpCode::GET_UPVALUE_LONG: {
                uint8_t slot = READ_BYTE();
                UpvalueObject* upvalue = frame->closure->getUpvalues()[slot];
                push(upvalue->isClosed() ?
                      upvalue->getClosed() :
                      m_stack[upvalue->getLocation()]);
                break;
            }

            case OpCode::SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                m_stack[frame->closure->getUpvalues()[slot]->getLocation()] = peek(0);
                break;
            }
            case OpCode::SET_UPVALUE_LONG: {
                uint32_t slot = READ_LONG();
                m_stack[frame->closure->getUpvalues()[slot]->getLocation()] = peek(0);
                break;
            }

            case OpCode::GET_PROPERTY: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint8_t index = READ_BYTE();
                push(instance->property(index));

                break;
            }
            case OpCode::GET_PROPERTY_LONG: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = READ_LONG();
                push(instance->property(index));

                break;
            }
            case OpCode::SET_PROPERTY: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint8_t index = READ_BYTE();
                instance->property(index) = peek(0);

                break;
            }
            case OpCode::SET_PROPERTY_LONG: {
                auto* instance = pop()
                        .asObject()
                        ->as<InstanceObject>();

                uint32_t index = READ_LONG();
                instance->property(index) = peek(0);

                break;
            }

            case OpCode::JUMP: {
                uint16_t jumpSize = READ_SHORT();
                frame->ip += jumpSize;
                break;
            }
            case OpCode::JUMP_IF_TRUE: {
                uint16_t jumpSize = READ_SHORT();
                if (peek(0).asBool()) {
                    frame->ip += jumpSize;
                }
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t jumpSize = READ_SHORT();
                if (!peek(0).asBool()) {
                    frame->ip += jumpSize;
                }
                break;
            }

            case OpCode::LOOP: {
                uint16_t jumpSize = READ_SHORT();
                frame->ip -= jumpSize;
                break;
            }

            case OpCode::CALL_FUNCTION: {
                uint8_t argCount = READ_BYTE();
                auto* closure = peek(argCount)
                        .asObject()
                        ->as<ClosureObject>();

                callFunction(closure, argCount);
                frame = &m_frames[m_frameCount - 1];
                break;
            }

            case OpCode::CALL_CONSTRUCTOR: {
                uint8_t argCount = READ_BYTE();
                auto* struct_ = peek(argCount)
                        .asObject()
                        ->as<StructObject>();

                callConstructor(struct_, argCount);
                break;
            }

            case OpCode::CALL_NATIVE: {
                uint8_t argCount = READ_BYTE();
                auto* native = peek(argCount)
                        .asObject()
                        ->as<NativeObject>();

                callNative(native, argCount);
                break;
            }

            case OpCode::CALL_DYNAMIC: {
                uint8_t argCount = READ_BYTE();
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
                    frame = &m_frames[m_frameCount - 1];
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
                FunctionObject* function = READ_CONSTANT().asObject()->as<FunctionObject>();
                push(Value{function});
                ClosureObject* closure = m_context.gc.allocateObject<ClosureObject>(function);
                pop();
                push(Value{closure});

                for (size_t i = 0; i < closure->getUpvalues().size(); ++i) {
                    uint8_t isLocal = READ_BYTE();
                    uint32_t index;
                    if (i < UINT8_MAX) {
                        index = READ_BYTE();
                    } else {
                        index = READ_LONG();
                    }

                    if (isLocal) {
                        closure->getUpvalues()[i] = captureUpvalue(frame->slotsBegin + index);
                    } else {
                        closure->getUpvalues()[i] = frame->closure->getUpvalues()[i];
                    }
                }
                break;
            }

            case OpCode::CLOSURE_LONG: {
                FunctionObject* function = READ_CONSTANT_LONG().asObject()->as<FunctionObject>();
                push(Value{function});
                ClosureObject* closure = m_context.gc.allocateObject<ClosureObject>(function);
                pop();
                push(Value{closure});

                for (size_t i = 0; i < closure->getUpvalues().size(); ++i) {
                    uint8_t isLocal = READ_BYTE();
                    uint32_t index;
                    if (i < UINT8_MAX) {
                        index = READ_BYTE();
                    } else {
                        index = READ_LONG();
                    }

                    if (isLocal) {
                        closure->getUpvalues()[i] = captureUpvalue(frame->slotsBegin + index);
                    } else {
                        closure->getUpvalues()[i] = frame->closure->getUpvalues()[i];
                    }
                }
                break;
            }

            case OpCode::CLOSE_UPVALUE:
                closeUpvalues(m_stack.size() - 1);
                pop();
                break;

            case OpCode::RETURN: {
                Value result = pop();

                closeUpvalues(frame->slotsBegin);

                m_frameCount--;
                if (m_frameCount == 0) {
                    pop();
                    return;
                }

                m_stack.erase(m_stack.begin() + frame->slotsBegin, m_stack.end());
                push(result);

                frame = &m_frames[m_frameCount - 1];
                break;
            }

            case OpCode::STRUCT: {
                std::shared_ptr<const ConstructorType> type{
                    READ_CONSTANT()
                        .asObject()
                        ->as<TypeObject>()
                        ->getContainedType()
                        ->as<ConstructorType>()
                };
                auto* struct_ = m_context.gc.allocateObject<StructObject>(type, std::vector<Value>{});

                push(Value{struct_});
                break;
            }
            case OpCode::STRUCT_LONG: {
                std::shared_ptr<const ConstructorType> type{
                    READ_CONSTANT_LONG()
                        .asObject()
                        ->as<TypeObject>()
                        ->getContainedType()
                        ->as<ConstructorType>()
                };
                auto* struct_ = m_context.gc.allocateObject<StructObject>(type, std::vector<Value>{});

                push(Value{struct_});
                break;
            }

            case OpCode::PAUSE: {
                m_pc = frame->ip - function->getChunk().getCode().data();
                m_frameCount--;
                return;
            }
        }

        #undef READ_BYTE
        #undef READ_SHORT
        #undef READ_LONG
        #undef READ_CONSTANT
        #undef READ_CONSTANT_LONG

        #undef NUMERIC_OP
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
            ->getProperties()
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