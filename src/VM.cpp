#include "h/VM.h"
#include "h/Enact.h"

VM::VM() : m_code{nullptr}, m_constants{nullptr}, m_stack{} {
}

InterpretResult VM::run(const Chunk& chunk) {
    m_code = &chunk.getCode();
    m_constants = &chunk.getConstants();

    auto ip = m_code->begin();

    for (size_t index = 0; ip != m_code->end(); ++index, ++ip) {
        #define READ_BYTE() (++index, *++ip)
        #define READ_LONG() (READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16))
        #define READ_CONSTANT() ((*m_constants)[READ_BYTE()])
        #define READ_CONSTANT_LONG() ((*m_constants)[READ_LONG()])
        #define ARITH_OP(op) \
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

        std::cout << chunk.disassembleInstruction(index).first;
        #endif

        switch (static_cast<OpCode>(*ip)) {
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
            case OpCode::FALSE: push(Value{}); break;
            case OpCode::NIL: push(Value{}); break;

            case OpCode::CHECK_NUMERIC: {
                Value value = pop();
                if (!value.isInt() && !value.isDouble()) {
                    runtimeError(chunk.getLine(index), "Expected a numeric value.");
                    return InterpretResult::RUNTIME_ERROR;
                }
            }

            case OpCode::ADD: ARITH_OP(+); break;
            case OpCode::SUBTRACT: ARITH_OP(-); break;
            case OpCode::MULTIPLY: ARITH_OP(*); break;
            case OpCode::DIVIDE: ARITH_OP(/); break;

            case OpCode::POP: pop(); break;

            case OpCode::RETURN:
                std::cout << pop() << "\n";
                return InterpretResult::OK;
        }

        #undef READ_BYTE
        #undef READ_LONG
        #undef READ_CONSTANT
        #undef READ_CONSTANT_LONG
    }
}

void VM::runtimeError(line_t line, const std::string& msg) {
    std::cerr << "[line " << line << "] Error:\n    " << Enact::getSourceLine(line) << msg;
}

void VM::push(Value value) {
    m_stack.push_back(value);
}

Value VM::pop() {
    ENACT_ASSERT(m_stack.size() > 0, "Stack underflow!");
    Value value = m_stack.back();
    m_stack.pop_back();
    return value;
}

Value VM::peek(size_t depth) {
    return m_stack[m_stack.size() - 1 - depth];
}
