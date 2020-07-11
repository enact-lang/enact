#ifndef ENACT_VARIABLEINFO_H
#define ENACT_VARIABLEINFO_H

namespace enact {
    struct VariableInfo {
        Type type;
        bool isMut;
        bool isInitialised;
    };
}

#endif //ENACT_VARIABLEINFO_H
