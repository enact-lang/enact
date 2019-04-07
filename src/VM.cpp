#include "h/VM.h"

VM::VM() : m_code{nullptr}, m_constants{nullptr}, m_stack{} {

}

InterpretResult VM::run(const Chunk& chunk) {
    m_code = &chunk.getCode();
    m_constants = &chunk.getConstants();

    auto ip = m_code->begin();

    for (size_t index = 0; ip != m_code->end(); ++index, ++ip) {
        #define READ_CONSTANT() ((*m_constants)[*(++ip)])

        #ifdef DEBUG_TRACE_EXECUTION
        std::cout << chunk.disassembleInstruction(index).first << "    [ ";
        for (Value value : m_stack) {
            std::cout << value << " ";
        }
        std::cout << "]\n";
        #endif

        switch ((OpCode)*ip) {
            case OpCode::CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OpCode::RETURN:
                std::cout << pop() << "\n";
                return InterpretResult::OK;
        }

        #undef READ_CONSTANT
    }
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
