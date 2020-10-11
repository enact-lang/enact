#ifndef ENACT_TYPEHANDLE_H
#define ENACT_TYPEHANDLE_H

#include <memory>

namespace enact {
    class Type;
    using TypeHandle = std::shared_ptr<Type>;

    template<class T>
    using TypeHandleOf = std::shared_ptr<T>;
}

#endif //ENACT_TYPEHANDLE_H
