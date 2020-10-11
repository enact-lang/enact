#ifndef ENACT_SEMADECLS_H
#define ENACT_SEMADECLS_H

#include <enact/ast/AstVisitor.h>

namespace enact {
    // Forward declared from "Sema.h"
    class Sema;

    // The first pass over the AST by Sema (semantic analysis).
    // We visit the top-level declarations provided to Sema by the parser, and declare
    // to Sema the variables and types that we come across - it's fine if they are
    // incomplete for now - we don't do any resolution until SemaDef, when everything
    // has been declared.
    class SemaDecls : private AstVisitor<void> {
    public:
        explicit SemaDecls(Sema& sema);

        // Walk the list of declarations provided by m_sema.
        void walk();

    private:
        void visitBreakStmt(BreakStmt& stmt) override;
        void visitContinueStmt(ContinueStmt& stmt) override;
        void visitEnumStmt(EnumStmt& stmt) override;
        void visitExpressionStmt(ExpressionStmt& stmt) override;
        void visitFunctionStmt(FunctionStmt& stmt) override;
        void visitImplStmt(ImplStmt& stmt) override;
        void visitReturnStmt(ReturnStmt& stmt) override;
        void visitStructStmt(StructStmt& stmt) override;
        void visitTraitStmt(TraitStmt& stmt) override;
        void visitVariableStmt(VariableStmt& stmt) override;

        void visitAssignExpr(AssignExpr& expr) override;
        void visitBinaryExpr(BinaryExpr& expr) override;
        void visitBlockExpr(BlockExpr& expr) override;
        void visitBooleanExpr(BooleanExpr& expr) override;
        void visitCallExpr(CallExpr& expr) override;
        void visitCastExpr(CastExpr& expr) override;
        void visitFloatExpr(FloatExpr& expr) override;
        void visitForExpr(ForExpr& expr) override;
        void visitGetExpr(FieldExpr& expr) override;
        void visitIfExpr(IfExpr& expr) override;
        void visitIntegerExpr(IntegerExpr& expr) override;
        void visitInterpolationExpr(InterpolationExpr& expr) override;
        void visitLogicalExpr(LogicalExpr& expr) override;
        void visitReferenceExpr(ReferenceExpr& expr) override;
        void visitStringExpr(StringExpr& expr) override;
        void visitSwitchExpr(SwitchExpr& expr) override;
        void visitSymbolExpr(SymbolExpr& expr) override;
        void visitTupleExpr(TupleExpr& expr) override;
        void visitUnaryExpr(UnaryExpr& expr) override;
        void visitUnitExpr(UnitExpr& expr) override;
        void visitWhileExpr(WhileExpr& expr) override;

        void visitValuePattern(ValuePattern& pattern) override;
        void visitWildcardPattern(WildcardPattern& pattern) override;

        Sema& m_sema;
    };
}

#endif //ENACT_SEMADECLS_H
