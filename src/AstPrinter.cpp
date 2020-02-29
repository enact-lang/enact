#include <sstream>
#include "h/AstPrinter.h"

void AstPrinter::print(Stmt stmt) {
    std::cout << evaluate(stmt) << "\n";
}

std::string AstPrinter::evaluate(Stmt stmt) {
    return stmt->accept(this);
}

std::string AstPrinter::evaluate(Expr expr) {
    return expr->accept(this);
}

std::string AstPrinter::visitBlockStmt(BlockStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Block [\n";
    for (auto &statement : stmt.statements) {
        s << evaluate(statement) << "\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitBreakStmt(BreakStmt& stmt) {
    return "Stmt::Break";
}

std::string AstPrinter::visitContinueStmt(ContinueStmt& stmt) {
    return "Stmt::Continue";
}

std::string AstPrinter::visitEachStmt(EachStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Each " << stmt.name.lexeme << " in " << evaluate(stmt.object) << " [\n";
    for (auto &statement : stmt.body) {
        s << evaluate(statement) << "\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitExpressionStmt(ExpressionStmt& stmt) {
    return "Stmt::Expression " + evaluate(stmt.expr);
}

std::string AstPrinter::visitForStmt(ForStmt& stmt) {
    std::stringstream s;
    s << "Stmt::For " << evaluate(stmt.initializer) << "; "
            << evaluate(stmt.condition) << "; "
            << evaluate(stmt.increment) << " [\n";
    for (auto &statement : stmt.body) {
        s << evaluate(statement) << "\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitFunctionStmt(FunctionStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Function " << stmt.name.lexeme << " (";

    std::string separator = "";
    for (auto &param : stmt.params) {
        s << separator << param.name.lexeme << " " << param.typeName;
        separator = ", ";
    }

    s << ") " << stmt.returnTypeName << " [\n";

    for (auto &statement : stmt.body) {
        s << evaluate(statement) << "\n";
    }

    s << "]";

    return s.str();
}

std::string AstPrinter::visitGivenStmt(GivenStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Given " << evaluate(stmt.value) << " [\n";
    for (auto &case_ : stmt.cases) {
        s << "when " << evaluate(case_.value) << " [\n";
        for (auto &statement: case_.body) {
            s << evaluate(statement) << "\n";
        }
        s << "]\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitIfStmt(IfStmt& stmt) {
    std::stringstream s;
    s << "Stmt::If " << evaluate(stmt.condition) << " [\n";
    for (auto &statement : stmt.thenBlock) {
        s << evaluate(statement) << "\n";
    }
    s << "] else [\n";
    for (auto &statement : stmt.elseBlock) {
        s << evaluate(statement) << "\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitReturnStmt(ReturnStmt& stmt) {
    return "Stmt::Return " + evaluate(stmt.value);
}

std::string AstPrinter::visitStructStmt(StructStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Struct " << stmt.name.lexeme << " ";

    std::string separator = "";
    for (auto &trait : stmt.traits) {
        s << separator << "is " << trait.lexeme;
        separator = ", ";
    }

    s << "[\n";

    for (auto &field : stmt.fields) {
        s << "field " << field.name.lexeme << " " << field.typeName << "\n";
    }

    for (auto &method : stmt.methods) {
        s << evaluate(method) << "\n";
    }

    for (auto &function : stmt.assocFunctions) {
        s << "assoc " << evaluate(function) << "\n";
    }

    s << "]";

    return s.str();
}

std::string AstPrinter::visitTraitStmt(TraitStmt& stmt) {
    std::stringstream s;
    s << "Stmt::Trait " << stmt.name.lexeme << " [\n";

    for (auto &method : stmt.methods) {
        s << evaluate(method) << "\n";
    }

    s << "]";

    return s.str();
}

std::string AstPrinter::visitWhileStmt(WhileStmt& stmt) {
    std::stringstream s;
    s << "Stmt::While " << evaluate(stmt.condition) << " then [\n";
    for (auto &statement : stmt.body) {
        s << evaluate(statement) << "\n";
    }
    s << "]";
    return s.str();
}

std::string AstPrinter::visitVariableStmt(VariableStmt& stmt) {
    return "Stmt::Var " + stmt.name.lexeme + " " + evaluate(stmt.initializer);
}

std::string AstPrinter::visitAnyExpr(AnyExpr& expr) {
    return "_";
}

std::string AstPrinter::visitArrayExpr(ArrayExpr& expr) {
    std::stringstream s;
    s << "[";

    std::string separator = "";
    for (auto &element : expr.value) {
        s << separator << evaluate(element);
        separator = ", ";
    }

    s << "] " << expr.getType()->toString();
    return s.str();
}

std::string AstPrinter::visitAssignExpr(AssignExpr& expr) {
    return "(= " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitBinaryExpr(BinaryExpr& expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitBooleanExpr(BooleanExpr& expr) {
    return (expr.value ? "true" : "false");
}

std::string AstPrinter::visitCallExpr(CallExpr& expr) {
    std::stringstream s;
    s << "(() " << evaluate(expr.callee);
    for (auto &arg : expr.arguments) {
        s << " " << evaluate(arg);
    }
    s << ")";
    return s.str();
}

std::string AstPrinter::visitFieldExpr(FieldExpr& expr) {
    return "(. " + evaluate(expr.object) + " " + expr.name.lexeme + ")";
}

std::string AstPrinter::visitFloatExpr(FloatExpr& expr) {
    return std::to_string(expr.value);
}

std::string AstPrinter::visitIntegerExpr(IntegerExpr& expr) {
    return std::to_string(expr.value);
}

std::string AstPrinter::visitLogicalExpr(LogicalExpr& expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.left) + " " + evaluate(expr.right) + ")";
}

std::string AstPrinter::visitNilExpr(NilExpr& expr) {
    return "nil";
}

std::string AstPrinter::visitReferenceExpr(ReferenceExpr &expr) {
    std::stringstream s;
    s << "&";
    if (expr.isVar) {
        s << "var";
    }
    s << " " << evaluate(expr.object);
    return s.str();
}

std::string AstPrinter::visitStringExpr(StringExpr& expr) {
    std::stringstream s;
    s << "\"" << expr.value << "\"";
    return s.str();
}

std::string AstPrinter::visitSubscriptExpr(SubscriptExpr& expr) {
    return "([] " + evaluate(expr.object) + " " + evaluate(expr.index) + ")";
}

std::string AstPrinter::visitTernaryExpr(TernaryExpr& expr) {
    return "(?: " + evaluate(expr.condition) + " " + evaluate(expr.thenExpr) + " " + evaluate(expr.elseExpr) + ")";
}

std::string AstPrinter::visitUnaryExpr(UnaryExpr& expr) {
    return "(" + expr.oper.lexeme + " " + evaluate(expr.operand) + ")";
}

std::string AstPrinter::visitVariableExpr(VariableExpr& expr) {
    return expr.name.lexeme;
}

