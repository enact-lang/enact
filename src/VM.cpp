#include <sstream>
#include "h/VM.h"
#include "h/Enact.h"
#include "h/GC.h"

VM::VM() : m_stack{} {
    GC::setVM(this);
}

InterpretResult VM::run(FunctionObject* function) {
    push(Value{function});

    CallFrame* frame = &m_frames[m_frameCount++];
    frame->closure = GC::allocateObject<ClosureObject>(function);
    pop();
    push(Value{frame->closure});
    frame->ip = function->getChunk().getCode().data();
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

        if (Enact::getFlags().flagEnabled(Flag::DEBUG_TRACE_EXECUTION)) {
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

            case OpCode::CHECK_NUMERIC: {
                Value value = peek(0);
                if (!value.getType()->isNumeric()) {
                    runtimeError("Expected a value of type 'int' or 'float', but got a value of type '"
                            + value.getType()->toString() + "' instead.");

                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::CHECK_BOOL: {
                Value value = peek(0);
                if (!value.getType()->isBool()) {
                    runtimeError("Expected a value of type 'bool', but got a value of type '"
                            + value.getType()->toString() + "' instead.");

                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::CHECK_REFERENCE: {
                Value value = peek(0);
                if (value.getType()->isPrimitive()) {
                    runtimeError("Only reference types can be copied, not a value of type '"
                                 + value.getType()->toString() + "'.");

                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::CHECK_CALLABLE: {
                uint8_t argCount = READ_BYTE();

                Value functionValue = peek(argCount);
                if (!functionValue.getType()->isFunction()) {
                    runtimeError("Only functions can be called, not a value of type '"
                            + functionValue.getType()->toString() + ".");
                    return InterpretResult::RUNTIME_ERROR;
                }

                const FunctionType* functionType = functionValue.getType()->as<FunctionType>();

                uint8_t paramCount = functionType->getArgumentTypes().size();
                if (argCount != paramCount) {
                    std::stringstream s;
                    s << "Expected " << static_cast<size_t>(paramCount) << " arguments to function, but got " <<
                            static_cast<size_t>(argCount) << ".";
                    runtimeError(s.str());
                    return InterpretResult::RUNTIME_ERROR;
                }

                for (uint8_t i = 0; i < argCount; ++i) {
                    Type shouldBe = functionType->getArgumentTypes()[i];
                    Type argumentType = peek(i).getType();
                    if (!argumentType->looselyEquals(*shouldBe)) {
                        std::stringstream s;
                        s << "Expected argument " << static_cast<size_t>(i) + 1 << " to be of type '" <<
                            shouldBe->toString() << "' but got value of type '" + argumentType->toString() <<
                            "' instead.";
                        runtimeError(s.str());
                        return InterpretResult::RUNTIME_ERROR;
                    }
                }
                break;
            }
            case OpCode::CHECK_TYPE: {
                Type shouldBe = READ_CONSTANT().asObject()->getType();
                Value value = peek(0);
                if (!shouldBe->looselyEquals(*value.getType())) {
                    runtimeError("Expected a value of type '" + shouldBe->toString() +
                            "' but got a value of type '" + value.getType()->toString() + "' instead.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                break;
            }
            case OpCode::CHECK_TYPE_LONG: {
                Type shouldBe = READ_CONSTANT_LONG().asObject()->getType();
                Value value = peek(0);
                if (!shouldBe->looselyEquals(*value.getType())) {
                    runtimeError("Expected a value of type '" + shouldBe->toString() +
                                 "' but got a value of type '" + value.getType()->toString() + "' instead.");
                    return InterpretResult::RUNTIME_ERROR;
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
                ArrayObject* array = GC::allocateObject<ArrayObject>(length);
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
                ArrayObject* array = GC::allocateObject<ArrayObject>(length);
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
                    runtimeError("Array index '" + std::to_string(index) + "' is out of bounds for array of "
                             + "length '" + std::to_string(array->asVector().size()) + "'.");
                    return InterpretResult::RUNTIME_ERROR;
                }

                push(array->at(index));
                break;
            }
            case OpCode::SET_ARRAY_INDEX: {
                int index = pop().asInt();
                ArrayObject* array = pop().asObject()->as<ArrayObject>();
                Value newValue = peek(0);

                if (index >= array->length()) {
                    runtimeError("Array index '" + std::to_string(index) + "' is out of bounds for array of "
                                 + "length '" + std::to_string(array->asVector().size()) + "'.");
                    return InterpretResult::RUNTIME_ERROR;
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

            case OpCode::CALL: {
                uint8_t argCount = READ_BYTE();
                Object* callee = peek(argCount).asObject();

                if (callee->is<ClosureObject>()) {
                    call(callee->as<ClosureObject>());
                    frame = &m_frames[m_frameCount - 1];
                } else {
                    NativeFn native = callee->as<NativeObject>()->getFunction();
                    Value result = native(argCount, &m_stack.back() - argCount + 1);

                    m_stack.erase(m_stack.end() - argCount - 1, m_stack.end());

                    push(result);
                }
                break;
            }

            case OpCode::CLOSURE: {
                FunctionObject* function = READ_CONSTANT().asObject()->as<FunctionObject>();
                push(Value{function});
                ClosureObject* closure = GC::allocateObject<ClosureObject>(function);
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
                ClosureObject* closure = GC::allocateObject<ClosureObject>(function);
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
                    return InterpretResult::OK;
                }

                m_stack.erase(m_stack.begin() + frame->slotsBegin, m_stack.end());
                push(result);

                frame = &m_frames[m_frameCount - 1];
                break;
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

void VM::call(ClosureObject* closure) {
    if (m_frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
    }

    CallFrame* frame = &m_frames[m_frameCount++];
    frame->closure = closure;
    frame->ip = closure->getFunction()->getChunk().getCode().data();

    uint8_t paramCount = closure->getFunction()->getType()->as<FunctionType>()->getArgumentTypes().size();
    frame->slotsBegin = m_stack.size() - paramCount - 1;
}

UpvalueObject* VM::captureUpvalue(uint32_t location) {
    UpvalueObject* prevUpvalue = nullptr;
    UpvalueObject* upvalue = m_openUpvalues;

    while (upvalue != nullptr && upvalue->getLocation() == location) {
        prevUpvalue = upvalue;
        upvalue = upvalue->getNext();
    }

    if (upvalue != nullptr && upvalue->getLocation() == location) return upvalue;

    auto* createdUpvalue = GC::allocateObject<UpvalueObject>(location);
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

void VM::runtimeError(const std::string& msg) {
    CallFrame* frame = &m_frames[m_frameCount - 1];
    size_t instruction = frame->ip - frame->closure->getFunction()->getChunk().getCode().data();
    line_t line = frame->closure->getFunction()->getChunk().getLine(instruction);

    const std::string source = Enact::getSourceLine(line);

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

}