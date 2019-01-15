#include <sstream>
#include "h/AstPrinter.h"

void AstPrinter::print(std::shared_ptr<Stmt> stmt) {
    std::cout << evaluate(stmt);
}

std::string AstPrinter::evaluate(std::shared_ptr<Stmt> stmt) {
    return stmt->accept(this);
}

std::string AstPrinter::evaluate(std::shared_ptr<Expr> expr) {
    return expr->accept(this);
}

std::string AstPrinter::visitBlockStmt(Stmt::Block stmt) {
    std::stringstream s;
    s << "Stmt::Block [\n";
    for (auto &statement : stmt.statements) {
        s << evaluate(statement) << "\n";
    }
    s << "]\n";
    return s.str();
}

std::string AstPrinter::visitExpressionStmt(Stmt::Expression stmt) {
    return "Stmt::Expression " + evaluate(stmt.expr);
}

std::string AstPrinter::visitIfStmt(Stmt::If stmt) {
    std::stringstream s;
    s << "Stmt::If (" << evaluate(stmt.condition) << ") then [\n";
    for (auto &statement : stmt.thenBlock) {
        s << evaluate(statement) << "\n";
    }
    s << "] else [\n";
    for (auto &statement : stmt.elseBlock) {
        s << evaluate(statement) << "\n";
    }
    s << "]\n";
    return s.str();
}

std::string AstPrinter::visitVariableStmt(Stmt::Variable stmt) {
    return "Stmt::Var " + stmt.name.lexeme + " " + evaluate(stmt.initializer);
}

std::string AstPrinter::visitArrayExpr(Expr::Array expr) {
    std::stringstream s;
    s << "[";

    std::string separator{""};
    for (auto &element : expr.value) {
        s << separator << evaluate(element);
        separator = ", ";
    }

    s << "]";
    return s.str();
}

std::string AstPrinter::visitAssignExpr(Expr::Assign expr) {
    return "(= " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitBinaryExpr(Expr::Binary expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitBooleanExpr(Expr::Boolean expr) {
    return (expr.value ? "true" : "false");
}

std::string AstPrinter::visitCallExpr(Expr::Call expr) {
    std::stringstream s;
    s << "(() " << evaluate(expr.callee);
    for (auto &arg : expr.arguments) {
        s << " " << evaluate(arg);
    }
    s << ")";
    return s.str();
}

std::string AstPrinter::visitFieldExpr(Expr::Field expr) {
    return "(. " + evaluate(expr.object) + " " + expr.name.lexeme + ")";
}

std::string AstPrinter::visitLogicalExpr(Expr::Logical expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitNilExpr(Expr::Nil expr) {
    return "nil";
}

std::string AstPrinter::visitReferenceExpr(Expr::Reference expr) {
    return "(ref " + evaluate(expr.object) + ")";
}

std::string AstPrinter::visitNumberExpr(Expr::Number expr) {
    std::stringstream s;
    s << expr.value;
    return s.str();
}

std::string AstPrinter::visitStringExpr(Expr::String expr) {
    std::stringstream s;
    s << "\"" << expr.value << "\"";
    return s.str();
}

std::string AstPrinter::visitSubscriptExpr(Expr::Subscript expr) {
    return "([] " + evaluate(expr.object) + " " + evaluate(expr.index) + ")";
}

std::string AstPrinter::visitTernaryExpr(Expr::Ternary expr) {
    return "(?: " + evaluate(expr.condition) + " " + evaluate(expr.thenExpr) + " " + evaluate(expr.elseExpr) + ")";
}

std::string AstPrinter::visitUnaryExpr(Expr::Unary expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.operand) + ")";
}

std::string AstPrinter::visitVariableExpr(Expr::Variable expr) {
    return expr.name.lexeme;
}

