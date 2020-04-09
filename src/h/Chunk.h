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

    CHECK_INT,
    CHECK_NUMERIC,
    CHECK_BOOL,
    CHECK_REFERENCE,
    CHECK_INDEXABLE,
    CHECK_ALLOTABLE,
    CHECK_TYPE,
    CHECK_TYPE_LONG,

    NEGATE,
    NOT,
    COPY,

    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,

    LESS,
    GREATER,
    EQUAL,

    ARRAY,
    ARRAY_LONG,

    GET_ARRAY_INDEX,
    SET_ARRAY_INDEX,

    POP,

    GET_LOCAL,
    GET_LOCAL_LONG,

    SET_LOCAL,
    SET_LOCAL_LONG,

    GET_UPVALUE,
    GET_UPVALUE_LONG,

    SET_UPVALUE,
    SET_UPVALUE_LONG,

    GET_FIELD,
    GET_FIELD_LONG,

    SET_FIELD,
    SET_FIELD_LONG,

    GET_METHOD,
    GET_METHOD_LONG,

    GET_PROPERTY_DYNAMIC,
    GET_PROPERTY_DYNAMIC_LONG,

    SET_PROPERTY_DYNAMIC,
    SET_PROPERTY_DYNAMIC_LONG,

    JUMP,
    JUMP_IF_TRUE,
    JUMP_IF_FALSE,
  
    LOOP,

    CALL_FUNCTION,
    CALL_CONSTRUCTOR,
    CALL_NATIVE,
    CALL_DYNAMIC,

    CLOSURE,
    CLOSURE_LONG,

    CLOSE_UPVALUE,
  
    RETURN,

    STRUCT,
    STRUCT_LONG,

    PAUSE,
};

std::string opCodeToString(OpCode code);

class Chunk {
    static constexpr size_t MAX_INSTRUCTION_NAME_LENGTH = 26;

    std::vector<uint8_t> m_code;
    std::vector<Value> m_constants;

    std::unordered_map<size_t, line_t> m_lines;

    std::pair<std::string, size_t> disassembleSimple(size_t index) const;
    std::pair<std::string, size_t> disassembleByte(size_t index) const;
    std::pair<std::string, size_t> disassembleShort(size_t index) const;
    std::pair<std::string, size_t> disassembleLong(size_t index) const;
    std::pair<std::string, size_t> disassembleConstant(size_t index, size_t argCount = 1) const;
    std::pair<std::string, size_t> disassembleLongConstant(size_t index, size_t argCount = 1) const;

    std::pair<std::string, size_t> disassembleClosure(size_t index, bool isLong) const;
    std::pair<std::string, size_t> disassembleStruct(size_t index, bool isLong) const;

    std::pair<std::string, size_t> disassembleClosureArgs(size_t index, bool isLong) const;

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
