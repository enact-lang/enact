#ifndef ENACT_ASTPRINTER_H
#define ENACT_ASTPRINTER_H

#include "../ast/Stmt.h"

#include <string>
#include <memory>

class AstPrinter : private Stmt::Visitor<std::string>, private Expr::Visitor<std::string> {
    std::string visitBlockStmt(Stmt::Block stmt) override;
    std::string visitExpressionStmt(Stmt::Expression stmt) override;
    std::string visitVariableStmt(Stmt::Variable stmt) override;

    std::string visitArrayExpr(Expr::Array expr) override;
    std::string visitAssignExpr(Expr::Assign expr) override;
    std::string visitBinaryExpr(Expr::Binary expr) override;
    std::string visitBooleanExpr(Expr::Boolean expr) override;
    std::string visitCallExpr(Expr::Call expr) override;
    std::string visitFieldExpr(Expr::Field expr) override;
    std::string visitLogicalExpr(Expr::Logical expr) override;
    std::string visitNilExpr(Expr::Nil expr) override;
    std::string visitNumberExpr(Expr::Number expr) override;
    std::string visitReferenceExpr(Expr::Reference expr) override;
    std::string visitStringExpr(Expr::String expr) override;
    std::string visitSubscriptExpr(Expr::Subscript expr) override;
    std::string visitTernaryExpr(Expr::Ternary expr) override;
    std::string visitUnaryExpr(Expr::Unary expr) override;
    std::string visitVariableExpr(Expr::Variable expr) override;

    std::string evaluate(std::shared_ptr<Stmt> stmt);
    std::string evaluate(std::shared_ptr<Expr> expr);
public:
    void print(std::shared_ptr<Stmt> stmt);
};

#endif //ENACT_ASTPRINTER_H
