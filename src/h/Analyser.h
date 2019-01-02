#ifndef ENACT_ANALYSER_H
#define ENACT_ANALYSER_H

#include "../ast/Expr.h"
#include "../ast/Stmt.h"

class Analyser : private Expr::Visitor<void>, private Stmt::Visitor<void> {
    void visitExpressionStmt(Stmt::Expression stmt) override;
    void visitPrintStmt(Stmt::Print stmt) override;
    void visitVariableStmt(Stmt::Variable stmt) override;

    void visitAssignExpr(Expr::Assign expr) override;
    void visitBinaryExpr(Expr::Binary expr) override;
    void visitBooleanExpr(Expr::Boolean expr) override;
    void visitCallExpr(Expr::Call expr) override;
    void visitNilExpr(Expr::Nil expr) override;
    void visitNumberExpr(Expr::Number expr) override;
    void visitStringExpr(Expr::String expr) override;
    void visitTernaryExpr(Expr::Ternary expr) override;
    void visitUnaryExpr(Expr::Unary expr) override;
    void visitVariableExpr(Expr::Variable expr) override;
public:
    void analyse(std::vector<Sp<Stmt>> statements);
};

#endif //ENACT_ANALYSER_H
