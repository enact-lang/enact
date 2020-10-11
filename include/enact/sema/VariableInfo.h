#ifndef ENACT_VARIABLEINFO_H
#define ENACT_VARIABLEINFO_H

#include <enact/sema/Type.h>

namespace enact {
    struct VariableInfo {
        TypeHandle type;
        bool isMut;
        bool isInitialised;
    };
}

#endif //ENACT_VARIABLEINFO_H
