#ifndef ENACT_ANALYSER_H
#define ENACT_ANALYSER_H

#include "../ast/Stmt.h"
#include "Type.h"

#include <map>

// Walks the AST and assigns a Type to each node.

class Analyser : private Stmt::Visitor<void>, private Expr::Visitor<void> {
    class AnalysisError : public std::runtime_error {
    public:
        AnalysisError() : std::runtime_error{"Uncaught AnalysisError: Internal"} {}
    };

    std::map<std::string, std::shared_ptr<Type>> m_variableTypes;
    bool m_hadError = false;

    void analyse(std::shared_ptr<Stmt> stmt);
    void analyse(std::shared_ptr<Expr> expr);

    AnalysisError errorAt(const Token &token, const std::string &message);

    void visitBlockStmt(Stmt::Block stmt) override;
    void visitBreakStmt(Stmt::Break stmt) override;
    void visitContinueStmt(Stmt::Continue stmt) override;
    void visitEachStmt(Stmt::Each stmt) override;
    void visitExpressionStmt(Stmt::Expression stmt) override;
    void visitForStmt(Stmt::For stmt) override;
    void visitFunctionStmt(Stmt::Function stmt) override;
    void visitGivenStmt(Stmt::Given stmt) override;
    void visitIfStmt(Stmt::If stmt) override;
    void visitReturnStmt(Stmt::Return stmt) override;
    void visitStructStmt(Stmt::Struct stmt) override;
    void visitTraitStmt(Stmt::Trait stmt) override;
    void visitWhileStmt(Stmt::While stmt) override;
    void visitVariableStmt(Stmt::Variable stmt) override;

    void visitAnyExpr(Expr::Any expr) override;
    void visitArrayExpr(Expr::Array expr) override;
    void visitAssignExpr(Expr::Assign expr) override;
    void visitBinaryExpr(Expr::Binary expr) override;
    void visitBooleanExpr(Expr::Boolean expr) override;
    void visitCallExpr(Expr::Call expr) override;
    void visitFieldExpr(Expr::Field expr) override;
    void visitFloatExpr(Expr::Float expr) override;
    void visitIntegerExpr(Expr::Integer expr) override;
    void visitLogicalExpr(Expr::Logical expr) override;
    void visitNilExpr(Expr::Nil expr) override;
    void visitReferenceExpr(Expr::Reference expr) override;
    void visitStringExpr(Expr::String expr) override;
    void visitSubscriptExpr(Expr::Subscript expr) override;
    void visitTernaryExpr(Expr::Ternary expr) override;
    void visitUnaryExpr(Expr::Unary expr) override;
    void visitVariableExpr(Expr::Variable expr) override;
public:
    void analyse(std::vector<std::shared_ptr<Stmt>> program);
};

#endif //ENACT_ANALYSER_H
