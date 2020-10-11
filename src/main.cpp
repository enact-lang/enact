//#include "../include/Enact.h"
#include "../include/enact/context/CompileContext.h"

int main(int argc, char *argv[]) {
    //enact::Options options{argc, argv};
    //enact::CompileContext context{options};

    //return static_cast<int>(context.run());

    enact::Options options{argc, argv};
    enact::CompileContext context{options};

    while (true) {
        std::cout << "enact > ";
        std::string input;
        std::getline(std::cin, input);

        context.compile(input);
    }
}