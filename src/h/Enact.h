#ifndef ENACT_ENACT_H
#define ENACT_ENACT_H

#include <string>
#include "VM.h"

enum class ExitCode {
    INVALID_ARGUMENTS = 65,
    FILE_ERROR = 70,
    COMPILE_ERROR = 75,
    STATIC_ERROR = 76,
    RUNTIME_ERROR = 77,
};

class Enact {
public:
    static void run(const std::string &source);
    static void runFile(const std::string &path);
    static void runPrompt();
    static void start(int argc, char *argv[]);
};

int main(int argc, char *argv[]);

#endif //ENACT_ENACT_H
