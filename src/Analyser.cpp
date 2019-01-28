#include "h/Analyser.h"
#include "h/Enact.h"

void Analyser::analyse(std::vector<std::shared_ptr<Stmt>> program) {
    for (auto &stmt : program) {
        analyse(stmt);
    }
}

void Analyser::analyse(std::shared_ptr<Stmt> stmt) {
    stmt->accept(this);
}

void Analyser::analyse(std::shared_ptr<Expr> expr) {
    expr->accept(this);
}

Analyser::AnalysisError Analyser::errorAt(const Token &token, const std::string &message) {
    Enact::reportErrorAt(token, message);
    m_hadError = true;
    return AnalysisError{};
}

void Analyser::visitBlockStmt(Stmt::Block stmt) {
    for (auto &statement : stmt.statements) {
        analyse(statement);
    }
}

void Analyser::visitBreakStmt(Stmt::Break stmt) {

}

void Analyser::visitContinueStmt(Stmt::Continue stmt) {

}

void Analyser::visitEachStmt(Stmt::Each stmt) {

}

void Analyser::visitExpressionStmt(Stmt::Expression stmt) {

}

void Analyser::visitForStmt(Stmt::For stmt) {

}

void Analyser::visitFunctionStmt(Stmt::Function stmt) {

}

void Analyser::visitGivenStmt(Stmt::Given stmt) {

}

void Analyser::visitIfStmt(Stmt::If stmt) {

}

void Analyser::visitReturnStmt(Stmt::Return stmt) {

}

void Analyser::visitStructStmt(Stmt::Struct stmt) {

}

void Analyser::visitTraitStmt(Stmt::Trait stmt) {

}

void Analyser::visitWhileStmt(Stmt::While stmt) {

}

void Analyser::visitVariableStmt(Stmt::Variable stmt) {

}

void Analyser::visitAnyExpr(Expr::Any expr) {
    expr.setType(Type{"_"});
}

void Analyser::visitArrayExpr(Expr::Array expr) {

}

void Analyser::visitAssignExpr(Expr::Assign expr) {

}

void Analyser::visitBinaryExpr(Expr::Binary expr) {

}

void Analyser::visitBooleanExpr(Expr::Boolean expr) {

}

void Analyser::visitCallExpr(Expr::Call expr) {
    analyse(expr.callee);
    if (expr.callee->type()->isFunction()) {
        auto calleeReturnType = expr.callee->type()->as<Type::Function>().getReturnType();
        expr.type() = std::make_shared<Type::Function>(calleeReturnType);
    } else if (expr.callee->type()->isDynamic()) {
        expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::DYNAMIC);
    } else {
        throw errorAt(expr.paren, "Only functions can be called.");
    }
}

void Analyser::visitFieldExpr(Expr::Field expr) {
    analyse(expr.object);
    if (expr.object->type()->isStruct()) {

    }
}

void Analyser::visitFloatExpr(Expr::Float expr) {
    expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::FLOAT);
}

void Analyser::visitIntegerExpr(Expr::Integer expr) {
    expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::INT);
}

void Analyser::visitLogicalExpr(Expr::Logical expr) {

}

void Analyser::visitNilExpr(Expr::Nil expr) {
    expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::NOTHING);
}

void Analyser::visitReferenceExpr(Expr::Reference expr) {

}

void Analyser::visitStringExpr(Expr::String expr) {
    expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::STRING);
}

void Analyser::visitSubscriptExpr(Expr::Subscript expr) {

}

void Analyser::visitTernaryExpr(Expr::Ternary expr) {

}

void Analyser::visitUnaryExpr(Expr::Unary expr) {
    analyse(expr.operand);
    switch (expr.oper.type) {
        case TokenType::BANG:
            expr.type() = std::make_shared<Type::Primitive>(Type::Primitive::Kind::BOOL);
            break;

        case TokenType::MINUS:
            if (!(expr.operand->type()->isNumeric() || expr.operand->type()->isDynamic())) {
                throw errorAt(expr.oper, "Only numbers can be negated.");
            }
            expr.type() = expr.operand->type();
            break;

        default: break; // Unreachable.
    }
}

void Analyser::visitVariableExpr(Expr::Variable expr) {
    if (m_variableTypes.count(expr.name.lexeme) > 0) {
        expr.type() = m_variableTypes[expr.name.lexeme];
    } else {
        throw errorAt(expr.name, "Undefined variable '" + expr.name.lexeme + "'.");
    }
}