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

std::string Chunk::disassemble() {
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

std::pair<std::string, size_t> Chunk::disassembleInstruction(size_t index) {
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
        case OpCode::RETURN: {
            std::string str;
            std::tie(str, index) = disassembleSimple(index);
            s << str;
            break;
        }
    }

    return std::pair(s.str(), index);
}

std::pair<std::string, size_t> Chunk::disassembleSimple(size_t index) {
    std::string s = opCodeToString(static_cast<OpCode>(m_code[index])) + "\n";
    return std::pair(s, ++index);
}

line_t Chunk::getLine(size_t index) {
    line_t line = 0;

    while (index >= 0 && m_lines.count(index) <= 0) {
        --index;
    }

    if (m_lines.count(index) > 0) {
        line = m_lines[index];
    }

    return line;
}

std::string opCodeToString(OpCode code) {
    switch (code) {
        case OpCode::RETURN: return "RETURN";
        // Unreachable.
        default: return "";
    }
}