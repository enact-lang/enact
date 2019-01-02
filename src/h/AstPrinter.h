#ifndef ENACT_ASTPRINTER_H
#define ENACT_ASTPRINTER_H

#include "../ast/Stmt.h"

#include <string>
#include <memory>

class AstPrinter : private Stmt::Visitor<std::string>, private Expr::Visitor<std::string> {
    std::string visitExpressionStmt(Stmt::Expression stmt) override;
    std::string visitPrintStmt(Stmt::Print stmt) override;
    std::string visitVariableStmt(Stmt::Variable stmt) override;

    std::string visitAssignExpr(Expr::Assign expr) override;
    std::string visitBinaryExpr(Expr::Binary expr) override;
    std::string visitBooleanExpr(Expr::Boolean expr) override;
    std::string visitCallExpr(Expr::Call expr) override;
    std::string visitNilExpr(Expr::Nil expr) override;
    std::string visitNumberExpr(Expr::Number expr) override;
    std::string visitStringExpr(Expr::String expr) override;
    std::string visitTernaryExpr(Expr::Ternary expr) override;
    std::string visitUnaryExpr(Expr::Unary expr) override;
    std::string visitVariableExpr(Expr::Variable expr) override;

    std::string evaluate(Sp<Expr> expr);
public:
    void print(Sp<Stmt> stmt);
};

#endif //ENACT_ASTPRINTER_H
