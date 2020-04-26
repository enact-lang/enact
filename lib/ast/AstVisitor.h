#ifndef ENACT_ASTVISITOR_H
#define ENACT_ASTVISITOR_H

#include "Stmt.h"

namespace enact {
    template<class T>
    class AstVisitor : public StmtVisitor<T>, public DeclVisitor<T>, public ExprVisitor<T> {};
}

#endif //ENACT_ASTVISITOR_H
