#ifndef ENACT_ASTVISITOR_H
#define ENACT_ASTVISITOR_H

#include "Stmt.h"

namespace enact {
    template<typename T>
    class AstVisitor : private StmtVisitor<T>, private ExprVisitor<T> {
    };
}

#endif //ENACT_ASTVISITOR_H
