#include <sstream>

#include "AstSerialise.h"

namespace enact {
    std::string AstSerialise::operator()(Stmt &stmt) {
        return visitStmt(stmt);
    }

    std::string AstSerialise::operator()(Expr &expr) {
        return visitExpr(expr);
    }

    std::string AstSerialise::visitBlockStmt(BlockStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::Block (\n";
        m_ident += "    ";

        for (auto &statement : stmt.statements) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitBreakStmt(BreakStmt &stmt) {
        return m_ident + "(Stmt::Break)";
    }

    std::string AstSerialise::visitContinueStmt(ContinueStmt &stmt) {
        return m_ident + "(Stmt::Continue)";
    }

    std::string AstSerialise::visitEachStmt(EachStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::Each (" << stmt.name.lexeme << " " << visitExpr(*stmt.object) << ") (\n";
        m_ident += "    ";

        for (auto &statement : stmt.body) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitExpressionStmt(ExpressionStmt &stmt) {
        return "(Stmt::Expression " + visitExpr(*stmt.expr) + ")";
    }

    std::string AstSerialise::visitForStmt(ForStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::For (" <<
                visitStmt(*stmt.initializer) << " " <<
                visitExpr(*stmt.condition) << " " <<
                visitExpr(*stmt.increment) << ") (\n";
        m_ident += "    ";

        for (auto &statement : stmt.body) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitFunctionStmt(FunctionStmt &stmt) {
        std::stringstream s;

        s << "(Stmt::Function " << stmt.name.lexeme << " (";

        for (auto &param : stmt.params) {
            s << param.name.lexeme << " " << param.typeName->name();
        }

        s << ") " << stmt.returnTypename->name() << " (\n";
        m_ident += "    ";

        for (auto &statement : stmt.body) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitGivenStmt(GivenStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::Given " << visitExpr(*stmt.value) << " (\n";
        m_ident += "    ";

        for (auto &case_ : stmt.cases) {
            s << m_ident << "(" << visitExpr(*case_.value) << " (\n";
            m_ident += "    ";

            for (auto &statement: case_.body) {
                s << visitStmt(*statement) << "\n";
            }

            m_ident.erase(m_ident.back() - 5);
            s << m_ident << ")\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitIfStmt(IfStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::If " << visitExpr(*stmt.condition) << " (\n";
        m_ident += "    ";

        for (auto &statement : stmt.thenBlock) {
            s << visitStmt(*statement) << "\n";
        }
        s << m_ident.substr(4) << ") (\n";
        for (auto &statement : stmt.elseBlock) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitReturnStmt(ReturnStmt &stmt) {
        return "(Stmt::Return " + visitExpr(*stmt.value) + ")";
    }

    std::string AstSerialise::visitStructStmt(StructStmt &stmt) {
        std::stringstream s;
        s << "(Stmt::Struct " << stmt.name.lexeme << " (";

        for (auto &trait : stmt.traits) {
            s << trait.lexeme;
        }

        s << ") (\n";
        m_ident += "    ";

        for (auto &field : stmt.fields) {
            s << m_ident << "(" << field.name.lexeme << " " << field.typeName->name() << ")\n";
        }

        for (auto &method : stmt.methods) {
            s << m_ident << visitStmt(*method) << "\n";
        }

        for (auto &function : stmt.assocFunctions) {
            s << m_ident << "(assoc " << visitStmt(*function) << ")\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitTraitStmt(TraitStmt &stmt) {
        std::stringstream s;

        s << "(Stmt::Trait " << stmt.name.lexeme << " (\n";
        m_ident += "    ";

        for (auto &method : stmt.methods) {
            s << m_ident << visitStmt(*method) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitVariableStmt(VariableStmt &stmt) {
        std::stringstream s;

        s << "(Stmt::Variable ";
        switch (stmt.mutability) {
            case Mutability::NONE:  s << "val "; break;
            case Mutability::BOXED: s << "let "; break;
            case Mutability::FULL:  s << "var "; break;
        }
        s << stmt.name.lexeme + " " + visitExpr(*stmt.initializer) << ")";

        return s.str();
    }

    std::string AstSerialise::visitWhileStmt(WhileStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::While " << visitExpr(*stmt.condition) << " (\n";
        m_ident += "    ";

        for (auto &statement : stmt.body) {
            s << visitStmt(*statement) << "\n";
        }

        m_ident.erase(m_ident.back() - 5);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitAllotExpr(AllotExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialise::visitAnyExpr(AnyExpr &expr) {
        return "_";
    }

    std::string AstSerialise::visitArrayExpr(ArrayExpr &expr) {
        std::stringstream s;
        s << "[";

        std::string separator;
        for (auto &element : expr.value) {
            s << separator << visitExpr(*element);
            separator = ", ";
        }

        s << "]";
        return s.str();
    }

    std::string AstSerialise::visitAssignExpr(AssignExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialise::visitBinaryExpr(BinaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialise::visitBooleanExpr(BooleanExpr &expr) {
        return (expr.value ? "true" : "false");
    }

    std::string AstSerialise::visitCallExpr(CallExpr &expr) {
        std::stringstream s;

        s << "(() " << visitExpr(*expr.callee);
        for (auto &arg : expr.arguments) {
            s << " " << visitExpr(*arg);
        }
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitFloatExpr(FloatExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialise::visitGetExpr(GetExpr &expr) {
        return "(. " + visitExpr(*expr.object) + " " + expr.name.lexeme + ")";
    }

    std::string AstSerialise::visitIntegerExpr(IntegerExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialise::visitLogicalExpr(LogicalExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialise::visitNilExpr(NilExpr &expr) {
        return "nil";
    }

    std::string AstSerialise::visitSetExpr(SetExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialise::visitStringExpr(StringExpr &expr) {
        std::stringstream s;
        s << "\"" << expr.value << "\"";
        return s.str();
    }

    std::string AstSerialise::visitSubscriptExpr(SubscriptExpr &expr) {
        return "([] " + visitExpr(*expr.object) + " " + visitExpr(*expr.index) + ")";
    }

    std::string AstSerialise::visitTernaryExpr(TernaryExpr &expr) {
        return "(?: " +
                visitExpr(*expr.condition) + " " +
                visitExpr(*expr.thenExpr) + " " +
                visitExpr(*expr.elseExpr) + ")";
    }

    std::string AstSerialise::visitUnaryExpr(UnaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.operand) + ")";
    }

    std::string AstSerialise::visitVariableExpr(VariableExpr &expr) {
        return expr.name.lexeme;
    }
}