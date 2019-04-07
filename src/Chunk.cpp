#include <sstream>
#include <iomanip>
#include <strings.h>
#include "h/Chunk.h"

void Chunk::write(uint8_t byte, line_t line) {
    m_code.push_back(byte);
    m_lines.insert(std::pair(m_code.size() - 1, line));
}

void Chunk::write(OpCode byte, line_t line) {
    m_code.push_back(static_cast<uint8_t>(byte));
    m_lines.insert(std::pair(m_code.size() - 1, line));
}

size_t Chunk::addConstant(Value constant) {
    m_constants.push_back(constant);
    return m_constants.size() - 1;
}

void Chunk::writeConstant(Value constant, line_t line) {
    size_t index = addConstant(constant);

    write(OpCode::CONSTANT, line);
    write(static_cast<uint8_t>(index), line);
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
        case OpCode::RETURN: {
            std::string str;
            std::tie(str, index) = disassembleSimple(index);
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
    }

    return {s.str(), index};
}

std::pair<std::string, size_t> Chunk::disassembleSimple(size_t index) const {
    std::string s = opCodeToString(static_cast<OpCode>(m_code[index])) + "\n";
    return {s, ++index};
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

const std::vector<uint8_t>& Chunk::getCode() const {
    return m_code;
}

const std::vector<Value>& Chunk::getConstants() const {
    return m_constants;
}

std::string opCodeToString(OpCode code) {
    switch (code) {
        case OpCode::CONSTANT: return "CONSTANT";
        case OpCode::RETURN: return "RETURN";
        // Unreachable.
        default: return "";
    }
}