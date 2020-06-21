#include <sstream>

#include "AstSerialize.h"

namespace enact {
    std::string AstSerialize::operator()(Stmt &stmt) {
        return visitStmt(stmt);
    }

    std::string AstSerialize::operator()(Expr &expr) {
        return visitExpr(expr);
    }

    std::string AstSerialize::visitBlockStmt(BlockStmt &stmt) {
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

    std::string AstSerialize::visitBreakStmt(BreakStmt &stmt) {
        return m_ident + "(Stmt::Break)";
    }

    std::string AstSerialize::visitContinueStmt(ContinueStmt &stmt) {
        return m_ident + "(Stmt::Continue)";
    }

    std::string AstSerialize::visitEachStmt(EachStmt &stmt) {
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

    std::string AstSerialize::visitExpressionStmt(ExpressionStmt &stmt) {
        return "(Stmt::Expression " + visitExpr(*stmt.expr) + ")";
    }

    std::string AstSerialize::visitForStmt(ForStmt &stmt) {
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

    std::string AstSerialize::visitFunctionStmt(FunctionStmt &stmt) {
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

    std::string AstSerialize::visitGivenStmt(GivenStmt &stmt) {
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

    std::string AstSerialize::visitIfStmt(IfStmt &stmt) {
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

    std::string AstSerialize::visitReturnStmt(ReturnStmt &stmt) {
        return "(Stmt::Return " + visitExpr(*stmt.value) + ")";
    }

    std::string AstSerialize::visitStructStmt(StructStmt &stmt) {
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

    std::string AstSerialize::visitTraitStmt(TraitStmt &stmt) {
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

    std::string AstSerialize::visitVariableStmt(VariableStmt &stmt) {
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

    std::string AstSerialize::visitWhileStmt(WhileStmt &stmt) {
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

    std::string AstSerialize::visitAllotExpr(AllotExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialize::visitAnyExpr(AnyExpr &expr) {
        return "_";
    }

    std::string AstSerialize::visitArrayExpr(ArrayExpr &expr) {
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

    std::string AstSerialize::visitAssignExpr(AssignExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialize::visitBinaryExpr(BinaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialize::visitBooleanExpr(BooleanExpr &expr) {
        return (expr.value ? "true" : "false");
    }

    std::string AstSerialize::visitCallExpr(CallExpr &expr) {
        std::stringstream s;

        s << "(() " << visitExpr(*expr.callee);
        for (auto &arg : expr.arguments) {
            s << " " << visitExpr(*arg);
        }
        s << ")";

        return s.str();
    }

    std::string AstSerialize::visitFloatExpr(FloatExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialize::visitGetExpr(GetExpr &expr) {
        return "(. " + visitExpr(*expr.object) + " " + expr.name.lexeme + ")";
    }

    std::string AstSerialize::visitIntegerExpr(IntegerExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialize::visitLogicalExpr(LogicalExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialize::visitNilExpr(NilExpr &expr) {
        return "nil";
    }

    std::string AstSerialize::visitSetExpr(SetExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialize::visitStringExpr(StringExpr &expr) {
        std::stringstream s;
        s << "\"" << expr.value << "\"";
        return s.str();
    }

    std::string AstSerialize::visitSubscriptExpr(SubscriptExpr &expr) {
        return "([] " + visitExpr(*expr.object) + " " + visitExpr(*expr.index) + ")";
    }

    std::string AstSerialize::visitTernaryExpr(TernaryExpr &expr) {
        return "(?: " +
                visitExpr(*expr.condition) + " " +
                visitExpr(*expr.thenExpr) + " " +
                visitExpr(*expr.elseExpr) + ")";
    }

    std::string AstSerialize::visitUnaryExpr(UnaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.operand) + ")";
    }

    std::string AstSerialize::visitVariableExpr(VariableExpr &expr) {
        return expr.name.lexeme;
    }
}