#ifndef ENACT_VM_H
#define ENACT_VM_H

#include "common.h"
#include "Value.h"
#include "Chunk.h"

#include <optional>

enum class InterpretResult {
    OK
};

class VM {
    const std::vector<uint8_t>* m_code;
    const std::vector<Value>* m_constants;

    std::vector<Value> m_stack;

public:
    VM();

    InterpretResult run(const Chunk& chunk);

    void push(Value value);
    Value pop();
    Value peek(size_t depth);
};

#endif //ENACT_VM_H
