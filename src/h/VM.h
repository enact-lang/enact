#ifndef ENACT_VM_H
#define ENACT_VM_H

#include "common.h"
#include "Value.h"
#include "Chunk.h"
#include "Object.h"

#include <optional>

constexpr size_t FRAMES_MAX = 64;

enum class InterpretResult {
    PARSE_ERROR,
    ANALYSIS_ERROR,
    COMPILE_ERROR,
    RUNTIME_ERROR,
    OK,
};

struct CallFrame {
    FunctionObject* function;
    const uint8_t* ip;
    size_t slotsBegin;
};

class VM {
    std::vector<Value> m_stack;

    std::array<CallFrame, FRAMES_MAX> m_frames{CallFrame{nullptr, nullptr, 0}};
    size_t m_frameCount = 0;

public:
    VM();

    InterpretResult run(FunctionObject* function);

    void push(Value value);
    Value pop();
    Value peek(size_t depth);

    void call(FunctionObject* function);

    void runtimeError(const std::string& msg);
};

#endif //ENACT_VM_H
