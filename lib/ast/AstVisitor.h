#ifndef ENACT_ASTVISITOR_H
#define ENACT_ASTVISITOR_H

#include "Stmt.h"

namespace enact {
    template<typename T>
    class AstVisitor : public StmtVisitor<T>, public ExprVisitor<T>, public PatternVisitor<T> {
    };
}

#endif //ENACT_ASTVISITOR_H
