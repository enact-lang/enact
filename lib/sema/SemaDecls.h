#ifndef ENACT_SEMADECLS_H
#define ENACT_SEMADECLS_H

#include "../ast/Stmt.h"

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

        std::vector<std::unique_ptr<Decl>> declare(std::vector<std::unique_ptr<Decl> ast);

    private:
        Sema& m_sema;

        void visitBlockStmt(BlockStmt &stmt) override;
        void visitBreakStmt(BreakStmt &stmt) override;
        void visitContinueStmt(ContinueStmt &stmt) override;
        void visitEachStmt(EachStmt &stmt) override;
        void visitDeclarationStmt(DeclarationStmt &stmt) override;
        void visitForStmt(ForStmt &stmt) override;
        void visitGivenStmt(GivenStmt &stmt) override;
        void visitIfStmt(IfStmt &stmt) override;
        void visitReturnStmt(ReturnStmt &stmt) override;
        void visitWhileStmt(WhileStmt &stmt) override;

        void visitExpressionDecl(ExpressionDecl& decl) override;
        void visitFunctionDecl(FunctionDecl& decl) override;
        void visitStructDecl(StructDecl& decl) override;
        void visitTraitDecl(TraitDecl& decl) override;
        void visitVariableDecl(VariableDecl& decl) override;

        void visitAllotExpr(AllotExpr &expr) override;
        void visitAnyExpr(AnyExpr &expr) override;
        void visitArrayExpr(ArrayExpr &expr) override;
        void visitAssignExpr(AssignExpr &expr) override;
        void visitBinaryExpr(BinaryExpr &expr) override;
        void visitBooleanExpr(BooleanExpr &expr) override;
        void visitCallExpr(CallExpr &expr) override;
        void visitFloatExpr(FloatExpr &expr) override;
        void visitGetExpr(GetExpr &expr) override;
        void visitIntegerExpr(IntegerExpr &expr) override;
        void visitLogicalExpr(LogicalExpr &expr) override;
        void visitNilExpr(NilExpr &expr) override;
        void visitSetExpr(SetExpr &expr) override;
        void visitStringExpr(StringExpr &expr) override;
        void visitSubscriptExpr(SubscriptExpr &expr) override;
        void visitTernaryExpr(TernaryExpr &expr) override;
        void visitUnaryExpr(UnaryExpr &expr) override;
        void visitVariableExpr(VariableExpr &expr) override;
    };
}

#endif //ENACT_SEMADECLS_H
