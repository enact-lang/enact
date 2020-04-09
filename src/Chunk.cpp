#include <sstream>
#include <iomanip>
#include <strings.h>
#include "h/Chunk.h"
#include "h/Object.h"

void Chunk::write(uint8_t byte, line_t line) {
    m_code.push_back(byte);
    m_lines.insert(std::pair(m_code.size() - 1, line));
}

void Chunk::write(OpCode byte, line_t line) {
    write(static_cast<uint8_t>(byte), line);
}

void Chunk::writeShort(uint32_t value, line_t line) {
    write(static_cast<uint8_t>(value & 0xff), line);
    write(static_cast<uint8_t>((value >> 8) & 0xff), line);
}

void Chunk::writeLong(uint32_t value, line_t line) {
    write(static_cast<uint8_t>(value & 0xff), line);
    write(static_cast<uint8_t>((value >> 8) & 0xff), line);
    write(static_cast<uint8_t>((value >> 16) & 0xff), line);
}

void Chunk::rewrite(size_t index, uint8_t byte) {
    m_code[index] = byte;
}

void Chunk::rewrite(size_t index, OpCode byte) {
    m_code[index] = static_cast<uint8_t>(byte);
}

size_t Chunk::addConstant(Value constant) {
    m_constants.push_back(constant);
    return m_constants.size() - 1;
}

void Chunk::writeConstant(Value constant, line_t line) {
    size_t index = addConstant(constant);

    if (index < UINT8_MAX) {
        write(OpCode::CONSTANT, line);
        write(static_cast<uint8_t>(index), line);
    } else {
        write(OpCode::CONSTANT_LONG, line);
        writeLong(static_cast<uint32_t>(index), line);
    }
}

std::string Chunk::disassemble() const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << "-- disassembly --\n";

    for (size_t i = 0; i < m_code.size();) {
        std::string str;
        std::tie(str, i) = disassembleInstruction(i);
        s << str;
    }

    return s.str();
}

std::pair<std::string, size_t> Chunk::disassembleInstruction(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::setfill('0') << std::setw(4) << index << "    ";
    s.flags(f);

    line_t line = getLine(index);
    if (index > 0 && line == getLine(index-1)) {
        s << "|";
    } else {
        s << line;
    }

    s << " ";

    auto op = static_cast<OpCode>(m_code[index]);
    switch (op) {
        // Simple instructions
        case OpCode::TRUE:
        case OpCode::FALSE:
        case OpCode::NIL:
        case OpCode::CHECK_INT:
        case OpCode::CHECK_NUMERIC:
        case OpCode::CHECK_BOOL:
        case OpCode::CHECK_REFERENCE:
        case OpCode::CHECK_INDEXABLE:
        case OpCode::CHECK_ALLOTABLE:
        case OpCode::NEGATE:
        case OpCode::NOT:
        case OpCode::COPY:
        case OpCode::ADD:
        case OpCode::SUBTRACT:
        case OpCode::MULTIPLY:
        case OpCode::DIVIDE:
        case OpCode::LESS:
        case OpCode::GREATER:
        case OpCode::EQUAL:
        case OpCode::GET_ARRAY_INDEX:
        case OpCode::SET_ARRAY_INDEX:
        case OpCode::POP:
        case OpCode::CLOSE_UPVALUE:
        case OpCode::RETURN:
        case OpCode::PAUSE: {
            std::string str;
            std::tie(str, index) = disassembleSimple(index);
            s << str;
            break;
        }

        // Byte instructions
        case OpCode::ARRAY:
        case OpCode::GET_LOCAL:
        case OpCode::SET_LOCAL:
        case OpCode::GET_UPVALUE:
        case OpCode::SET_UPVALUE:
        case OpCode::GET_FIELD:
        case OpCode::SET_FIELD:
        case OpCode::GET_METHOD:
        case OpCode::GET_ASSOC:
        case OpCode::CALL_FUNCTION:
        case OpCode::CALL_BOUND_METHOD:
        case OpCode::CALL_CONSTRUCTOR:
        case OpCode::CALL_NATIVE:
        case OpCode::CALL_DYNAMIC: {
            std::string str;
            std::tie(str, index) = disassembleByte(index);
            s << str;
            break;
        }

        // Short instructions
        case OpCode::JUMP:
        case OpCode::JUMP_IF_TRUE:
        case OpCode::JUMP_IF_FALSE:
        case OpCode::LOOP: {
            std::string str;
            std::tie(str, index) = disassembleShort(index);
            s << str;
            break;
        }

        // Long instructions
        case OpCode::ARRAY_LONG:
        case OpCode::GET_LOCAL_LONG:
        case OpCode::SET_LOCAL_LONG:
        case OpCode::GET_UPVALUE_LONG:
        case OpCode::SET_UPVALUE_LONG:
        case OpCode::GET_FIELD_LONG:
        case OpCode::SET_FIELD_LONG:
        case OpCode::GET_METHOD_LONG:
        case OpCode::GET_ASSOC_LONG: {
            std::string str;
            std::tie(str, index) = disassembleLong(index);
            s << str;
            break;
        }

        // Constant instructions
        case OpCode::CONSTANT:
        case OpCode::CHECK_TYPE:
        case OpCode::GET_PROPERTY_DYNAMIC:
        case OpCode::SET_PROPERTY_DYNAMIC: {
            std::string str;
            std::tie(str, index) = disassembleConstant(index);
            s << str;
            break;
        }

        // Long constant instructions
        case OpCode::CONSTANT_LONG:
        case OpCode::CHECK_TYPE_LONG:
        case OpCode::GET_PROPERTY_DYNAMIC_LONG:
        case OpCode::SET_PROPERTY_DYNAMIC_LONG: {
            std::string str;
            std::tie(str, index) = disassembleLongConstant(index);
            s << str;
            break;
        }

        // Struct instructions
        case OpCode::STRUCT: {
            std::string str;
            std::tie(str, index) = disassembleStruct(index, false);
            s << str;
            break;
        }

        // Long struct instructions
        case OpCode::STRUCT_LONG: {
            std::string str;
            std::tie(str, index) = disassembleStruct(index, true);
            s << str;
            break;
        }

        // Closure instructions
        case OpCode::CLOSURE: {
            std::string str{};
            std::tie(str, index) = disassembleClosure(index, false);
            s << str;
            break;
        }
        case OpCode::CLOSURE_LONG: {
            std::string str{};
            std::tie(str, index) = disassembleClosure(index, true);
            s << str;
            break;
        }
    }

    return {s.str(), index};
}

std::pair<std::string, size_t> Chunk::disassembleSimple(size_t index) const {
    std::string s = opCodeToString(static_cast<OpCode>(m_code[index])) + "\n";
    return {s, ++index};
}

std::pair<std::string, size_t> Chunk::disassembleByte(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the byte argument
    uint8_t arg = m_code[++index];
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleShort(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the long argument
    uint16_t arg = (m_code[++index] | (m_code[++index] << 8));
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleLong(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the long argument
    uint32_t arg = (m_code[++index] | (m_code[++index] << 8) | (m_code[++index] << 16));
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleConstant(size_t index, size_t argCount) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    for (size_t i = argCount; i <= argCount; ++i) {
        size_t constant = m_code[++index];

        s << " " << constant << " (";
        s << m_constants[constant] << ")\n";
    }

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleLongConstant(size_t index, size_t argCount) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    for (size_t i = argCount; i <= argCount; ++i) {
        size_t constant = m_code[index + 1] |
                          (m_code[index + 2] << 8) |
                          (m_code[index + 3] << 16);

        index += 3;

        s << " " << constant << " (";
        s << m_constants[constant] << ")\n";
    }

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleClosure(size_t index, bool isLong) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << std::setfill(' ') << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    std::string tmp;
    std::tie(tmp, index) = disassembleClosureArgs(index, isLong);
    s << tmp;

    return {s.str(), index};
}

std::pair<std::string, size_t> Chunk::disassembleStruct(size_t index, bool isLong) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(MAX_INSTRUCTION_NAME_LENGTH) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    size_t constant;
    if (isLong) {
        constant = m_code[index + 1] |
                    (m_code[index + 2] << 8) |
                    (m_code[index + 3] << 16);
        index += 3;
    } else {
        constant = m_code[++index];
    }

    s << " " << constant << " (";
    s << m_constants[constant] << ")\n";

    auto type = std::static_pointer_cast<const ConstructorType>(
            m_constants[constant]
                    .asObject()
                    ->as<TypeObject>()
                    ->getContainedType()
    );

    uint32_t methodCount = type
            ->getStructType()
            ->getMethods()
            .length();

    for (uint32_t i = 0; i < methodCount; ++i) {
        std::string methodName = type->getStructType()->getMethods().keys()[i];
        s << "          | method '" << methodName << "'";

        std::string tmp;
        std::tie(tmp, index) = disassembleClosureArgs(index, isLong);
        s << tmp;
    }

    uint32_t assocCount = type
            ->getAssocProperties()
            .length();

    for (uint32_t i = 0; i < assocCount; ++i) {
        std::string assocName = type->getAssocProperties().keys()[i];
        s << "          | assoc '" << assocName << "'\n";
        disassembleClosureArgs(index, isLong);
    }

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleClosureArgs(size_t index, bool isLong) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    size_t constant;
    if (isLong) {
        constant = m_code[index + 1] |
            (m_code[index + 2] << 8) |
            (m_code[index + 3] << 16);
        index += 3;
    } else {
        constant = m_code[++index];
    }

    s << " " << constant << " (";
    s << m_constants[constant] << ")\n";

    auto* function = m_constants[constant].asObject()->as<FunctionObject>();
    for (int j = 0; j < function->getUpvalueCount(); j++) {
        uint8_t isLocal = m_code[++index];
        uint32_t i;
        if (j < UINT8_MAX) {
            i = m_code[++index];
        } else {
            i = m_code[index + 1] |
                (m_code[index + 2] << 8) |
                (m_code[index + 3] << 16);
            index += 3;
        }
        s << std::setfill('0') << std::setw(4) << index - 2;
        s.flags(f);
        s << "       | " << (isLocal ? "local" : "upvalue") << " " << i << "\n";
    }

    return {s.str(), index};
}

line_t Chunk::getLine(size_t index) const {
    line_t line = 0;

    while (index >= 0 && m_lines.count(index) <= 0) {
        --index;
    }

    if (m_lines.count(index) > 0) {
        line = m_lines.find(index)->second;
    }

    return line;
}

line_t Chunk::getCurrentLine() const {
    line_t line = 1;
    if (m_code.size() > 1) {
        line = getLine(m_code.size() - 2);
    }

    return line;
}

const std::vector<uint8_t>& Chunk::getCode() const {
    return m_code;
}

const std::vector<Value>& Chunk::getConstants() const {
    return m_constants;
}

size_t Chunk::getCount() const {
    return m_code.size();
}

std::string opCodeToString(OpCode code) {
    switch (code) {
        case OpCode::CONSTANT: return "CONSTANT";
        case OpCode::CONSTANT_LONG: return "CONSTANT_LONG";
        case OpCode::TRUE: return "TRUE";
        case OpCode::FALSE: return "FALSE";
        case OpCode::NIL: return "NIL";
        case OpCode::CHECK_INT: return "CHECK_INT";
        case OpCode::CHECK_NUMERIC: return "CHECK_NUMERIC";
        case OpCode::CHECK_BOOL: return "CHECK_BOOL";
        case OpCode::CHECK_REFERENCE: return "CHECK_REFERENCE";
        case OpCode::CHECK_INDEXABLE: return "CHECK_INDEXABLE";
        case OpCode::CHECK_ALLOTABLE: return "CHECK_ALLOTABLE";
        case OpCode::CHECK_TYPE: return "CHECK_TYPE";
        case OpCode::CHECK_TYPE_LONG: return "CHECK_TYPE_LONG";
        case OpCode::NEGATE: return "NEGATE";
        case OpCode::NOT: return "NOT";
        case OpCode::COPY: return "COPY";
        case OpCode::ADD: return "ADD";
        case OpCode::SUBTRACT: return "SUBTRACT";
        case OpCode::MULTIPLY: return "MULTIPLY";
        case OpCode::DIVIDE: return "DIVIDE";
        case OpCode::LESS: return "LESS";
        case OpCode::GREATER: return "GREATER";
        case OpCode::EQUAL: return "EQUAL";
        case OpCode::ARRAY: return "ARRAY";
        case OpCode::ARRAY_LONG: return "ARRAY_LONG";
        case OpCode::GET_ARRAY_INDEX: return "GET_ARRAY_INDEX";
        case OpCode::SET_ARRAY_INDEX: return "SET_ARRAY_INDEX";
        case OpCode::POP: return "POP";
        case OpCode::GET_LOCAL: return "GET_LOCAL";
        case OpCode::GET_LOCAL_LONG: return "GET_LOCAL_LONG";
        case OpCode::SET_LOCAL: return "SET_LOCAL";
        case OpCode::SET_LOCAL_LONG: return "SET_LOCAL_LONG";
        case OpCode::GET_UPVALUE: return "GET_UPVALUE";
        case OpCode::GET_UPVALUE_LONG: return "GET_UPVALUE_LONG";
        case OpCode::SET_UPVALUE: return "SET_UPVALUE";
        case OpCode::SET_UPVALUE_LONG: return "SET_UPVALUE_LONG";
        case OpCode::GET_FIELD: return "GET_FIELD";
        case OpCode::GET_FIELD_LONG: return "GET_FIELD_LONG";
        case OpCode::SET_FIELD: return "SET_FIELD";
        case OpCode::SET_FIELD_LONG: return "SET_FIELD_LONG";
        case OpCode::GET_METHOD: return "GET_METHOD";
        case OpCode::GET_METHOD_LONG: return "GET_METHOD_LONG";
        case OpCode::GET_ASSOC: return "GET_ASSOC";
        case OpCode::GET_ASSOC_LONG: return "GET_ASSOC_LONG";
        case OpCode::GET_PROPERTY_DYNAMIC: return "GET_PROPERTY_DYNAMIC";
        case OpCode::GET_PROPERTY_DYNAMIC_LONG: return "GET_PROPERTY_DYNAMIC_LONG";
        case OpCode::SET_PROPERTY_DYNAMIC: return "SET_PROPERTY_DYNAMIC";
        case OpCode::SET_PROPERTY_DYNAMIC_LONG: return "SET_PROPERTY_DYNAMIC_LONG";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_TRUE: return "JUMP_IF_TRUE";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::LOOP: return "LOOP";
        case OpCode::CALL_FUNCTION: return "CALL_FUNCTION";
        case OpCode::CALL_BOUND_METHOD: return "CALL_BOUND_METHOD";
        case OpCode::CALL_CONSTRUCTOR: return "CALL_CONSTRUCTOR";
        case OpCode::CALL_NATIVE: return "CALL_NATIVE";
        case OpCode::CALL_DYNAMIC: return "CALL_DYNAMIC";
        case OpCode::CLOSURE: return "CLOSURE";
        case OpCode::CLOSURE_LONG: return "CLOSURE_LONG";
        case OpCode::CLOSE_UPVALUE: return "CLOSE_UPVALUE";
        case OpCode::RETURN: return "RETURN";
        case OpCode::STRUCT: return "STRUCT";
        case OpCode::STRUCT_LONG: return "STRUCT_LONG";
        case OpCode::PAUSE: return "PAUSE";
        // Unreachable.
        default: return "";
    }
}