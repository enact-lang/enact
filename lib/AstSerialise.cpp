#include <sstream>

#include "AstSerialise.h"

namespace enact {
    std::string AstSerialise::operator()(Stmt &stmt) {
        return visitStmt(stmt);
    }

    std::string AstSerialise::operator()(Expr &expr) {
        return visitExpr(expr);
    }

    std::string AstSerialise::visitBreakStmt(BreakStmt &stmt) {
        return m_ident + "(Stmt::Break)";
    }

    std::string AstSerialise::visitContinueStmt(ContinueStmt &stmt) {
        return m_ident + "(Stmt::Continue)";
    }

    std::string AstSerialise::visitEnumStmt(EnumStmt &stmt) {
        std::stringstream s;
        s << "(Stmt::Enum " << stmt.name.lexeme << " (";

        s << ") (\n";
        m_ident += "    ";

        for (const EnumStmt::Variant& variant : stmt.variants) {
            s << m_ident << "(" << variant.name.lexeme << " " << variant.typename_->name() << ")\n";
        }

        m_ident.erase(0, 4);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitExpressionStmt(ExpressionStmt &stmt) {
        return m_ident + "(Stmt::Expression " + visitExpr(*stmt.expr) + ")";
    }

    std::string AstSerialise::visitFunctionStmt(FunctionStmt &stmt) {
        std::stringstream s;

        s << "(Stmt::Function " << stmt.name.lexeme << " (";

        std::string separator;
        for (auto &param : stmt.params) {
            s << separator << param.name.lexeme << ' ' << param.typename_->name();
            separator = " ";
        }

        s << ") " << stmt.returnTypename->name() << " (\n";
        m_ident += "    ";

        s << visitExpr(*stmt.body);

        m_ident.erase(0, 4);
        s << ')';

        return s.str();
    }

    std::string AstSerialise::visitImplStmt(ImplStmt &stmt) {
        std::stringstream s;
        s << m_ident << "(Stmt::Impl " << stmt.typename_->name() << ' ';

        if (stmt.traitTypename != nullptr) {
            s << stmt.traitTypename->name() << ' ';
        }

        s << "(\n";
        m_ident += "    ";

        for (const std::unique_ptr<FunctionStmt>& method : stmt.methods) {
            s << visitFunctionStmt(*method);
        }

        m_ident.erase(0, 4);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitReturnStmt(ReturnStmt &stmt) {
        return "(Stmt::Return " + visitExpr(*stmt.value) + ")";
    }

    std::string AstSerialise::visitStructStmt(StructStmt &stmt) {
        std::stringstream s;
        s << m_ident << "(Stmt::Struct " << stmt.name.lexeme << " (";

        s << ") (\n";
        m_ident += "    ";

        for (auto &field : stmt.fields) {
            s << m_ident << "(" << field.name.lexeme << " " << field.typename_->name() << ")\n";
        }

        m_ident.erase(0, 4);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitTraitStmt(TraitStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::Trait " << stmt.name.lexeme << " (\n";
        m_ident += "    ";

        for (auto &method : stmt.methods) {
            s << m_ident << visitStmt(*method) << "\n";
        }

        m_ident.erase(0, 4);
        s << ")";

        return s.str();
    }

    std::string AstSerialise::visitVariableStmt(VariableStmt &stmt) {
        std::stringstream s;

        s << m_ident << "(Stmt::Variable ";
        s << stmt.keyword.lexeme << ' ';
        s << stmt.name.lexeme + " " + visitExpr(*stmt.initializer) << ")";

        return s.str();
    }

    std::string AstSerialise::visitAssignExpr(AssignExpr &expr) {
        return "(= " + visitExpr(*expr.target) + " " + visitExpr(*expr.value) + ")";
    }

    std::string AstSerialise::visitBinaryExpr(BinaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialise::visitBlockExpr(BlockExpr &expr) {
        std::stringstream s;

        s << m_ident << "(Expr::Block (\n";
        m_ident += "    ";

        for (auto &statement : expr.stmts) {
            s << m_ident << visitStmt(*statement) << '\n';
        }
        s << m_ident << visitExpr(*expr.expr) << ')';

        m_ident.erase(0, 4);

        return s.str();
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

    std::string AstSerialise::visitCastExpr(CastExpr& expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.expr) + " " + expr.typename_->name() + ")";
    }

    std::string AstSerialise::visitFloatExpr(FloatExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialise::visitForExpr(ForExpr& expr) {
        std::stringstream s;

        s << m_ident << "(Expr::For (" << expr.name.lexeme << " " << visitExpr(*expr.object) << ")\n";
        m_ident += "    ";

        s << visitExpr(*expr.body);

        m_ident.erase(0, 4);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitGetExpr(FieldExpr &expr) {
        return "(. " + visitExpr(*expr.object) + " " + expr.name.lexeme + ")";
    }

    std::string AstSerialise::visitIfExpr(IfExpr& expr) {
        std::stringstream s;

        s << m_ident << "(Expr::If " << visitExpr(*expr.condition) << '\n';
        m_ident += "    ";

        s << visitExpr(*expr.thenBody) << '\n';
        s << visitExpr(*expr.elseBody) << ')';

        m_ident.erase(0, 4);

        return s.str();
    }

    std::string AstSerialise::visitIntegerExpr(IntegerExpr &expr) {
        return std::to_string(expr.value);
    }

    std::string AstSerialise::visitInterpolationExpr(InterpolationExpr &expr) {
        return visitExpr(*expr.start) + "\\(" + visitExpr(*expr.interpolated) + ")" + visitExpr(*expr.end);
    }

    std::string AstSerialise::visitLogicalExpr(LogicalExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.left) + " " + visitExpr(*expr.right) + ")";
    }

    std::string AstSerialise::visitStringExpr(StringExpr &expr) {
        std::stringstream s;
        s << "\"" << expr.value << "\"";
        return s.str();
    }

    std::string AstSerialise::visitSwitchExpr(SwitchExpr& expr) {
        std::stringstream s;

        s << m_ident << "(Expr::Switch " << visitExpr(*expr.value) << " (\n";
        m_ident += "    ";

        for (auto &case_ : expr.cases) {
            s << m_ident << "(" << visitPattern(*case_.pattern) << " " <<
                    visitExpr(*case_.predicate) << " (\n";
            m_ident += "    ";

            s << visitExpr(*case_.body);

            m_ident.erase(0, 4);
            s << m_ident << ")\n";
        }

        m_ident.erase(0, 4);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitSymbolExpr(SymbolExpr& expr) {
        return expr.name.lexeme;
    }

    std::string AstSerialise::visitTupleExpr(TupleExpr &expr) {
        std::ostringstream s;
        s << '(';

        std::string separator;
        for (const std::unique_ptr<Expr>& elem : expr.elems) {
            s << separator << visitExpr(*elem);
            separator = ", ";
        }

        s << ')';
        return s.str();
    }

    std::string AstSerialise::visitUnaryExpr(UnaryExpr &expr) {
        return "(" + expr.oper.lexeme + " " + visitExpr(*expr.operand) + ")";
    }

    std::string AstSerialise::visitUnitExpr(UnitExpr &expr) {
        return "()";
    }

    std::string AstSerialise::visitWhileExpr(WhileExpr& expr) {
        std::stringstream s;

        s << m_ident << "(Expr::While " << visitExpr(*expr.condition) << '\n';
        m_ident += "    ";

        s << visitExpr(*expr.body);

        m_ident.erase(0, 4);
        s << m_ident << ")";

        return s.str();
    }

    std::string AstSerialise::visitValuePattern(ValuePattern &pattern) {
        return visitExpr(*pattern.value);
    }

    std::string AstSerialise::visitWildcardPattern(WildcardPattern &pattern) {
        return "_";
    }
}