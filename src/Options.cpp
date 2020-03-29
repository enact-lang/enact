#include <iostream>
#include "h/Options.h"

Options::Options(std::string filename, std::vector<std::string> programArgs, std::unordered_set<Flag> flags) :
        m_filename{std::move(filename)},
        m_programArgs{std::move(programArgs)},
        m_flags{std::move(flags)} {
}

Options::Options(int argc, char **argv) {
    if (argc == 0) return;
    std::vector<std::string> args{argv + 1, argv + argc};

    size_t current = 0;
    for (; current < args.size(); ++current) {
        std::string arg = args[current];

        if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
            parseString(arg);
        } else if (arg.size() >= 1 && arg[0] == '-') {
            for (char c : arg.substr(1)) {
                parseString(std::string{"-"} + c);
            }
        } else {
            break;
        }
    }

    if (current >= args.size()) {
        // There are no more arguments for us to parse.
        return;
    }

    m_filename = args[current++];

    while (current < args.size()) {
        m_programArgs.push_back(args[current++]);
    }
}

void Options::parseString(const std::string& string) {
    if (m_parseTable.count(string) > 0) {
        m_parseTable[string]();
    } else {
        std::cerr << "[enact] Error:\n    Unknown interpreter flag '" << string <<
                "'.\nUsage: enact [interpreter flags] [filename] [program flags]\n\n";
        throw FlagsError{};
    }
}

void Options::parseStrings(const std::vector<std::string>& strings) {
    for (const std::string& string : strings) {
        parseString(string);
    }
}

bool Options::flagEnabled(Flag flag) const {
    return m_flags.count(flag) > 0;
}

void Options::enableFlag(Flag flag) {
    m_flags.insert(flag);
}

void Options::enableFlags(std::vector<Flag> flags) {
    for (Flag flag : flags) {
        enableFlag(flag);
    }
}

const std::string& Options::getFilename() {
    return m_filename;
}

const std::vector<std::string>& Options::getProgramArgs() {
    return m_programArgs;
}
