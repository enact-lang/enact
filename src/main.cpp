#include "../include/Enact.h"

int main(int argc, char *argv[]) {
    enact::Options options{argc, argv};
    enact::Context context{options};

    return static_cast<int>(context.run());
}