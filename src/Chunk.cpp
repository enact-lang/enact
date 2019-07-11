#include <sstream>
#include <iomanip>
#include <strings.h>
#include "h/Chunk.h"

void Chunk::write(uint8_t byte, line_t line) {
    m_code.push_back(byte);
    m_lines.insert(std::pair(m_code.size() - 1, line));
}

void Chunk::write(OpCode byte, line_t line) {
    write(static_cast<uint8_t>(byte), line);
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
        case OpCode::CHECK_NUMERIC:
        case OpCode::CHECK_BOOL:
        case OpCode::ADD:
        case OpCode::SUBTRACT:
        case OpCode::MULTIPLY:
        case OpCode::DIVIDE:
        case OpCode::POP:
        case OpCode::RETURN: {
            std::string str;
            std::tie(str, index) = disassembleSimple(index);
            s << str;
            break;
        }

        // Byte instructions
        case OpCode::GET_VARIABLE:
        case OpCode::SET_VARIABLE: {
            std::string str;
            std::tie(str, index) = disassembleByte(index);
            s << str;
            break;
        }

        // Short instructions
        case OpCode::JUMP:
        case OpCode::JUMP_IF_FALSE: {
            std::string str;
            std::tie(str, index) = disassembleShort(index);
            s << str;
            break;
        }

        // Long instructions
        case OpCode::GET_VARIABLE_LONG:
        case OpCode::SET_VARIABLE_LONG: {
            std::string str;
            std::tie(str, index) = disassembleLong(index);
            s << str;
            break;
        }

        // Constant instructions
        case OpCode::CONSTANT: {
            std::string str;
            std::tie(str, index) = disassembleConstant(index);
            s << str;
            break;
        }

        // Long constant instructions
        case OpCode::CONSTANT_LONG: {
            std::string str;
            std::tie(str, index) = disassembleLongConstant(index);
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

    s << std::left << std::setw(16) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the byte argument
    uint8_t arg = m_code[++index];
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleShort(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(16) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the long argument
    uint16_t arg = (m_code[++index] | (m_code[++index] << 8));
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleLong(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(16) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    // Output the long argument
    uint32_t arg = (m_code[++index] | (m_code[++index] << 8) | (m_code[++index] << 16));
    s << " " << static_cast<size_t>(arg) << "\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleConstant(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(16) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    size_t constant = m_code[++index];

    s << " " << constant << " (";
    s << m_constants[constant] << ")\n";

    return {s.str(), ++index};
}

std::pair<std::string, size_t> Chunk::disassembleLongConstant(size_t index) const {
    std::stringstream s;
    std::ios_base::fmtflags f( s.flags() );

    s << std::left << std::setw(16) << opCodeToString(static_cast<OpCode>(m_code[index]));
    s.flags(f);

    size_t constant =  m_code[index + 1] |
                       (m_code[index + 2] << 8) |
                       (m_code[index + 3] << 16);

    index += 3;

    s << " " << constant << " (";
    s << m_constants[constant] << ")\n";

    return {s.str(), ++index};
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
        case OpCode::CHECK_NUMERIC: return "CHECK_NUMERIC";
        case OpCode::CHECK_BOOL: return "CHECK_BOOL";
        case OpCode::ADD: return "ADD";
        case OpCode::SUBTRACT: return "SUBTRACT";
        case OpCode::MULTIPLY: return "MULTIPLY";
        case OpCode::DIVIDE: return "DIVIDE";
        case OpCode::POP: return "POP";
        case OpCode::GET_VARIABLE: return "GET_VARIABLE";
        case OpCode::GET_VARIABLE_LONG: return "GET_VARIABLE_LONG";
        case OpCode::SET_VARIABLE: return "SET_VARIABLE";
        case OpCode::SET_VARIABLE_LONG: return "SET_VARIABLE_LONG";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::RETURN: return "RETURN";
        // Unreachable.
        default: return "";
    }
}