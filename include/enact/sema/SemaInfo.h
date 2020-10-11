#ifndef ENACT_SEMAINFO_H
#define ENACT_SEMAINFO_H

#include <enact/sema/TypeHandle.h>

namespace enact {
    struct SemaInfo {
        TypeHandle type;
        bool isAssignable;
    };
}

#endif //ENACT_SEMAINFO_H
