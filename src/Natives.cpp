#include "bytecode/Chunk.h"
#include "value/Object.h"

#include "Natives.h"

Value Natives::print(uint8_t argCount, Value* args) {
    std::cout << args[0] << "\n";
    return Value{};
}

Value Natives::put(uint8_t argCount, Value* args) {
    std::cout << args[0];
    return Value{};
}

Value Natives::dis(uint8_t count, Value* args) {
    Chunk& chunk = args[0].asObject()->as<ClosureObject>()->getFunction()->getChunk();
    return Value{new StringObject{chunk.disassemble()}};
}