#ifndef ENACT_ASTVISITOR_H
#define ENACT_ASTVISITOR_H

#include "Stmt.h"

namespace enact {
    template<class T>
    class AstVisitor : public StmtVisitor<T>, public DeclVisitor<T>, public ExprVisitor<T> {
    public:
        virtual T visit(Stmt& stmt) {
            return stmt.accept(*this);
        }

        virtual T visit(Decl& decl) {
            return decl.accept(*this);
        }

        virtual T visit(Expr& expr) {
            return expr.accept(*this);
        }
    };
}

#endif //ENACT_ASTVISITOR_H
