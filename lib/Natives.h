#ifndef ENACT_NATIVES_H
#define ENACT_NATIVES_H

#include "value/Value.h"

namespace enact {
    namespace Natives {
        Value print(uint8_t argCount, Value *args);
        Value put(uint8_t argCount, Value *args);
        Value dis(uint8_t argCount, Value *args);
    }
}

#endif //ENACT_NATIVES_H
