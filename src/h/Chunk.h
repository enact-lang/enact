#ifndef ENACT_CHUNK_H
#define ENACT_CHUNK_H

#include <vector>
#include "common.h"
#include "Value.h"

enum class OpCode : uint8_t {
    RETURN,
};

std::string opCodeToString(OpCode code);

class Chunk {
    std::vector<uint8_t> m_code;
    std::vector<Value> m_constants;

    std::unordered_map<size_t, line_t> m_lines;

    std::pair<std::string, size_t> disassembleSimple(size_t index);

public:

    Chunk() = default;

    void write(uint8_t byte, line_t line);
    void write(OpCode byte, line_t line);

    std::string disassemble();
    std::pair<std::string, size_t> disassembleInstruction(size_t index);

    line_t getLine(size_t index);
};

#endif //ENACT_CHUNK_H
