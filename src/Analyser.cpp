#include "h/Analyser.h"
/*
void Analyser::analyse(std::vector<Sp<Stmt>> statements) {

}

void Analyser::visitExpressionStmt(Stmt::Expression stmt) {

}

void Analyser::visitVariableStmt(Stmt::Variable stmt) {

}

void Analyser::visitAssignExpr(Expr::Assign expr) {

}

void Analyser::visitBinaryExpr(Expr::Binary expr) {

}

void Analyser::visitBooleanExpr(Expr::Boolean expr) {

}

void Analyser::visitCallExpr(Expr::Call expr) {

}

void Analyser::visitNilExpr(Expr::Nil expr) {

<<<<<<< Updated upstream
=======
    if (!stmt.condition->getType()->maybeBool()) {
        throw errorAt(stmt.keyword, "While condition must be a bool.");
    }

    m_insideLoop = true;
    beginScope();
    for (Stmt &statement : stmt.body) {
        analyse(statement);
    }
    endScope();
    m_insideLoop = false;
>>>>>>> Stashed changes
}

void Analyser::visitNumberExpr(Expr::Number expr) {

}

void Analyser::visitStringExpr(Expr::String expr) {

}

void Analyser::visitTernaryExpr(Expr::Ternary expr) {

}

void Analyser::visitUnaryExpr(Expr::Unary expr) {

}

void Analyser::visitVariableExpr(Expr::Variable expr) {

}
*/