#ifndef ENACT_ITEM_H
#define ENACT_ITEM_H

#include <enact/sema/TypeHandle.h>

// An item defined on a type
// Could be a field or a method
struct Item {
    enum class Visibility {
        PRIVATE,
        PUBLIC // denoted with 'pub'
    };

    Visibility visibility;
    TypeHandle type;
};

#endif //ENACT_ITEM_H
