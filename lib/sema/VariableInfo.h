#ifndef ENACT_VARIABLEINFO_H
#define ENACT_VARIABLEINFO_H

namespace enact {
    enum class Mutability {
        NONE,  // 'val'
        BOXED, // 'let'
        FULL   // 'var'
    };

    struct VariableInfo {
        Type type;
        Mutability mutability;
        bool isInitialised;
    };
}

#endif //ENACT_VARIABLEINFO_H
