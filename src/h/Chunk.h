#ifndef ENACT_CHUNK_H
#define ENACT_CHUNK_H

#include <vector>
#include "common.h"
#include "Value.h"

enum class OpCode : uint8_t {
    CONSTANT,
    CONSTANT_LONG,

    TRUE,
    FALSE,
    NIL,

    CHECK_NUMERIC,
    CHECK_BOOL,

    NEGATE,
    NOT,

    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,

    LESS,
    GREATER,
    EQUAL,

    POP,

    GET_VARIABLE,
    GET_VARIABLE_LONG,

    SET_VARIABLE,
    SET_VARIABLE_LONG,

    JUMP,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
  
    LOOP,
  
    RETURN,
};

std::string opCodeToString(OpCode code);

class Chunk {
    std::vector<uint8_t> m_code;
    std::vector<Value> m_constants;

    std::unordered_map<size_t, line_t> m_lines;

    std::pair<std::string, size_t> disassembleSimple(size_t index) const;
    std::pair<std::string, size_t> disassembleByte(size_t index) const;
    std::pair<std::string, size_t> disassembleShort(size_t index) const;
    std::pair<std::string, size_t> disassembleLong(size_t index) const;
    std::pair<std::string, size_t> disassembleConstant(size_t index) const;
    std::pair<std::string, size_t> disassembleLongConstant(size_t index) const;

public:
    Chunk() = default;

    void write(uint8_t byte, line_t line);
    void write(OpCode byte, line_t line);

    void writeShort(uint32_t value, line_t line);

    void writeLong(uint32_t value, line_t line);

    void writeConstant(Value constant, line_t line);
    size_t addConstant(Value constant);

    void rewrite(size_t index, uint8_t byte);
    void rewrite(size_t index, OpCode byte);

    std::string disassemble() const;
    std::pair<std::string, size_t> disassembleInstruction(size_t index) const;

    line_t getLine(size_t index) const;
    line_t getCurrentLine() const;

    const std::vector<uint8_t>& getCode() const;
    const std::vector<Value>& getConstants() const;

    size_t getCount() const;
};

#endif //ENACT_CHUNK_H
