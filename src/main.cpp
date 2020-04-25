#include "../lib/context/Context.h"

int main(int argc, char *argv[]) {
    Options options{argc, argv};
    Context context{options};

    return static_cast<int>(context.run());
}