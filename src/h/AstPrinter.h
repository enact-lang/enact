#ifndef ENACT_ASTPRINTER_H
#define ENACT_ASTPRINTER_H

#include "../ast/Stmt.h"

#include <string>
#include <memory>

class AstPrinter : private StmtVisitor<std::string>, private ExprVisitor<std::string> {
    std::string visitBlockStmt(BlockStmt& stmt) override;
    std::string visitBreakStmt(BreakStmt& stmt) override;
    std::string visitContinueStmt(ContinueStmt& stmt) override;
    std::string visitEachStmt(EachStmt& stmt) override;
    std::string visitExpressionStmt(ExpressionStmt& stmt) override;
    std::string visitForStmt(ForStmt& stmt) override;
    std::string visitFunctionStmt(FunctionStmt& stmt) override;
    std::string visitGivenStmt(GivenStmt& stmt) override;
    std::string visitIfStmt(IfStmt& stmt) override;
    std::string visitReturnStmt(ReturnStmt& stmt) override;
    std::string visitStructStmt(StructStmt& stmt) override;
    std::string visitTraitStmt(TraitStmt& stmt) override;
    std::string visitWhileStmt(WhileStmt& stmt) override;
    std::string visitVariableStmt(VariableStmt& stmt) override;

    std::string visitAllotExpr(AllotExpr& expr) override;
    std::string visitAnyExpr(AnyExpr& expr) override;
    std::string visitArrayExpr(ArrayExpr& expr) override;
    std::string visitAssignExpr(AssignExpr& expr) override;
    std::string visitBinaryExpr(BinaryExpr& expr) override;
    std::string visitBooleanExpr(BooleanExpr& expr) override;
    std::string visitCallExpr(CallExpr& expr) override;
    std::string visitFloatExpr(FloatExpr& expr) override;
    std::string visitGetExpr(GetExpr& expr) override;
    std::string visitIntegerExpr(IntegerExpr& expr) override;
    std::string visitLogicalExpr(LogicalExpr& expr) override;
    std::string visitNilExpr(NilExpr& expr) override;
    std::string visitSetExpr(SetExpr& expr) override;
    std::string visitStringExpr(StringExpr& expr) override;
    std::string visitSubscriptExpr(SubscriptExpr& expr) override;
    std::string visitTernaryExpr(TernaryExpr& expr) override;
    std::string visitUnaryExpr(UnaryExpr& expr) override;
    std::string visitVariableExpr(VariableExpr& expr) override;

    std::string evaluate(Stmt& stmt);
    std::string evaluate(Expr& expr);
public:
    void print(Stmt& stmt);
};

#endif //ENACT_ASTPRINTER_H
