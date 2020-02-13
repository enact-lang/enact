#include "h/Natives.h"

Value Natives::print(uint8_t argCount, Value* args) {
    std::cout << args[0] << "\n";
    return Value{};
}

Value Natives::put(uint8_t argCount, Value* args) {
    std::cout << args[0];
    return Value{};
}