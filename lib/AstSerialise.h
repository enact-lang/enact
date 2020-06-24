#ifndef ENACT_ASTSERIALISE_H
#define ENACT_ASTSERIALISE_H

#include "ast/AstVisitor.h"

namespace enact {
    // A functor which takes an AST node (Stmt/Decl/Expr), serializes it,
    // and returns the output as an std::string.
    class AstSerialise : private AstVisitor<std::string> {
        std::string m_ident = "";

        std::string visitBreakStmt(BreakStmt& stmt) override;
        std::string visitContinueStmt(ContinueStmt& stmt) override;
        std::string visitExpressionStmt(ExpressionStmt& stmt) override;
        std::string visitFunctionStmt(FunctionStmt& decl) override;
        std::string visitReturnStmt(ReturnStmt& stmt) override;
        std::string visitStructStmt(StructStmt& stmt) override;
        std::string visitTraitStmt(TraitStmt& stmt) override;
        std::string visitVariableStmt(VariableStmt& stmt) override;

        std::string visitAssignExpr(AssignExpr& expr) override;
        std::string visitBinaryExpr(BinaryExpr& expr) override;
        std::string visitBlockExpr(BlockExpr &expr) override;
        std::string visitBooleanExpr(BooleanExpr& expr) override;
        std::string visitCallExpr(CallExpr& expr) override;
        std::string visitFloatExpr(FloatExpr& expr) override;
        std::string visitForExpr(ForExpr &expr) override;
        std::string visitGetExpr(GetExpr& expr) override;
        std::string visitIfExpr(IfExpr &expr) override;
        std::string visitIntegerExpr(IntegerExpr& expr) override;
        std::string visitLogicalExpr(LogicalExpr& expr) override;
        std::string visitStringExpr(StringExpr& expr) override;
        std::string visitSwitchExpr(SwitchExpr &expr) override;
        std::string visitSymbolExpr(SymbolExpr &expr) override;
        std::string visitUnaryExpr(UnaryExpr& expr) override;
        std::string visitWhileExpr(WhileExpr &expr) override;

    public:
        std::string operator()(Stmt& stmt);
        std::string operator()(Expr& expr);
    };
}

#endif //ENACT_ASTSERIALISE_H
