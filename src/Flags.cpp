#include <iostream>
#include "h/Flags.h"

Flags::Flags(std::unordered_set<Flag> flags) : m_flags{std::move(flags)} {
}

Flags::Flags(const std::vector<std::string>& strings) : m_flags{} {
    parseStrings(strings);
}

void Flags::parseString(const std::string& string) {
    if (m_parseTable.count(string) > 0) {
        m_parseTable[string]();
    } else {
        std::cerr << "[enact] Error:\n    Unknown interpreter flag '" << string <<
                "'.\nUsage: enact [interpreter flags] [filename] [program flags]\n\n";
        m_hadError = true;
    }
}

void Flags::parseStrings(const std::vector<std::string>& strings) {
    for (const std::string& string : strings) {
        parseString(string);
    }
}

bool Flags::flagEnabled(Flag flag) const {
    return m_flags.count(flag) > 0;
}

void Flags::enableFlag(Flag flag) {
    m_flags.insert(flag);
}

void Flags::enableFlags(std::vector<Flag> flags) {
    for (Flag flag : flags) {
        enableFlag(flag);
    }
}

bool Flags::hadError() {
    return m_hadError;
}
