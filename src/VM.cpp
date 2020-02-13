#include "h/VM.h"
#include "h/Enact.h"

VM::VM() : m_stack{} {
}

InterpretResult VM::run(FunctionObject* function) {
    push(Value{function});

    CallFrame* frame = &m_frames[m_frameCount++];
    frame->function = function;
    frame->ip = function->getChunk().getCode().data();
    frame->slotsBegin = 0;

    for (;;) {
        #define READ_BYTE() (*frame->ip++)
        #define READ_SHORT() (static_cast<uint16_t>(READ_BYTE() | (READ_BYTE() << 8)))
        #define READ_LONG() (static_cast<uint32_t>(READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16)))
        #define READ_CONSTANT() ((frame->function->getChunk().getConstants())[READ_BYTE()])
        #define READ_CONSTANT_LONG() ((frame->function->getChunk().getConstants())[READ_LONG()])
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

        #ifdef DEBUG_TRACE_EXECUTION
        std::cout << "    ";
        for (Value value : m_stack) {
            std::cout << "[ " << value << " ] ";
        }
        std::cout << "\n";

        std::cout << frame->function->getChunk()
                .disassembleInstruction(frame->ip - frame->function->getChunk().getCode().data()).first;
        #endif

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

            case OpCode::POP: pop(); break;

            case OpCode::GET_VARIABLE: {
                uint8_t slot = READ_BYTE();
                push(slots[slot]);
                break;
            }
            case OpCode::GET_VARIABLE_LONG: push(slots[READ_LONG()]); break;

            case OpCode::SET_VARIABLE: slots[READ_BYTE()] = peek(0); break;
            case OpCode::SET_VARIABLE_LONG: slots[READ_LONG()] = peek(0); break;

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
                Value functionValue = peek(argCount);

                if (functionValue.asObject()->is<FunctionObject>()) {
                    call(functionValue.asObject()->as<FunctionObject>());
                    frame = &m_frames[m_frameCount - 1];
                } else {
                    NativeFn native = functionValue.asObject()->as<NativeObject>()->getFunction();
                    Value result = native(argCount, &m_stack.back() - argCount + 1);

                    m_stack.erase(m_stack.begin() + frame->slotsBegin + argCount + 1, m_stack.end());

                    push(result);
                }
                break;
            }

            case OpCode::RETURN: {
                Value result = pop();

                m_frameCount--;
                if (m_frameCount == 0) {
                    pop();
                    return InterpretResult::OK;
                }

                m_stack.erase(m_stack.begin() + frame->slotsBegin + 1, m_stack.end());
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

void VM::call(FunctionObject* function) {
    if (m_frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
    }

    CallFrame* frame = &m_frames[m_frameCount++];
    frame->function = function;
    frame->ip = function->getChunk().getCode().data();

    uint8_t paramCount = function->getType()->as<FunctionType>()->getArgumentTypes().size();
    frame->slotsBegin = m_stack.size() - paramCount - 1;
}

void VM::runtimeError(const std::string& msg) {
    CallFrame* frame = &m_frames[m_frameCount - 1];
    size_t instruction = frame->ip - frame->function->getChunk().getCode().data();
    line_t line = frame->function->getChunk().getLine(instruction);

    const std::string source = Enact::getSourceLine(line);

    std::cerr << "[line " << line << "] Error on this line:\n    " << source << "\n    ";
    for (int i = 0; i < source.size(); ++i) {
        std::cerr << "^";
    }
    std::cerr << "\n" << msg << "\n";
    for (uint8_t i = m_frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &m_frames[i];
        FunctionObject* function = frame->function;

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