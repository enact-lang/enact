// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include <memory>
#include <vector>

#include "../parser/Typename.h"

#include "Pattern.h"

namespace enact {
    // From "Stmt.h":
    class Stmt;

    template<class R>
    class ExprVisitor;

    class Expr {
    public:
        //SemaInfo semaInfo{};

        virtual ~Expr() = default;

        // We need to overload for every possible visitor return type here, as we cannot
        // have a templated virtual member function.
        virtual std::string accept(ExprVisitor<std::string> *visitor) = 0;
        virtual void accept(ExprVisitor<void> *visitor) = 0;
    };

    class AssignExpr;
    class BinaryExpr;
    class BlockExpr;
    class BooleanExpr;
    class CallExpr;
    class FloatExpr;
    class ForExpr;
    class GetExpr;
    class IfExpr;
    class IntegerExpr;
    class LogicalExpr;
    class StringExpr;
    class SwitchExpr;
    class SymbolExpr;
    class UnaryExpr;
    class UnitExpr;
    class WhileExpr;

    template<class R>
    class ExprVisitor {
    public:
        R visitExpr(Expr& expr) {
            return expr.accept(*this);
        }

        virtual R visitAssignExpr(AssignExpr &expr) = 0;
        virtual R visitBinaryExpr(BinaryExpr &expr) = 0;
        virtual R visitBlockExpr(BlockExpr &expr) = 0;
        virtual R visitBooleanExpr(BooleanExpr &expr) = 0;
        virtual R visitCallExpr(CallExpr &expr) = 0;
        virtual R visitFloatExpr(FloatExpr &expr) = 0;
        virtual R visitForExpr(ForExpr &expr) = 0;
        virtual R visitGetExpr(GetExpr &expr) = 0;
        virtual R visitIfExpr(IfExpr &expr) = 0;
        virtual R visitIntegerExpr(IntegerExpr &expr) = 0;
        virtual R visitLogicalExpr(LogicalExpr &expr) = 0;
        virtual R visitStringExpr(StringExpr &expr) = 0;
        virtual R visitSwitchExpr(SwitchExpr &expr) = 0;
        virtual R visitSymbolExpr(SymbolExpr &expr) = 0;
        virtual R visitUnaryExpr(UnaryExpr &expr) = 0;
        virtual R visitUnitExpr(UnitExpr& expr) = 0;
        virtual R visitWhileExpr(WhileExpr &expr) = 0;
    };

    class AssignExpr : public Expr {
    public:
        std::unique_ptr<Expr> target;
        std::unique_ptr<Expr> value;
        Token oper;

        AssignExpr(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value, Token oper) :
                target{std::move(target)},
                value{std::move(value)},
                oper{std::move(oper)} {}

        ~AssignExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitAssignExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitAssignExpr(*this);
        }
    };

    class BinaryExpr : public Expr {
    public:
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
        Token oper;

        BinaryExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Token oper) :
                left{std::move(left)},
                right{std::move(right)},
                oper{std::move(oper)} {}

        ~BinaryExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitBinaryExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitBinaryExpr(*this);
        }
    };

    class BlockExpr : public Expr {
    public:
        // Statements preceeding the final expression
        std::vector<std::unique_ptr<Stmt>> stmts;

        // The final expression
        std::unique_ptr<Expr> expr;

        BlockExpr(std::vector<std::unique_ptr<Stmt>> stmts, std::unique_ptr<Expr> expr) :
                stmts{std::move(stmts)},
                expr{std::move(expr)} {}

        ~BlockExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitBlockExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitBlockExpr(*this);
        }
    };

    class BooleanExpr : public Expr {
    public:
        bool value;

        explicit BooleanExpr(bool value) :
                value{value} {}

        ~BooleanExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitBooleanExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitBooleanExpr(*this);
        }
    };

    class CallExpr : public Expr {
    public:
        std::unique_ptr<Expr> callee;
        std::vector<std::unique_ptr<Expr>> arguments;
        Token paren;

        CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments, Token paren) :
                callee{std::move(callee)},
                arguments{std::move(arguments)},
                paren{std::move(paren)} {}

        ~CallExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitCallExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitCallExpr(*this);
        }
    };

    class FloatExpr : public Expr {
    public:
        double value;

        explicit FloatExpr(double value) :
                value{value} {}

        ~FloatExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitFloatExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitFloatExpr(*this);
        }
    };

    class ForExpr : public Expr {
    public:
        Token name;
        std::unique_ptr<Expr> object;
        std::unique_ptr<BlockExpr> body;

        ForExpr(Token name, std::unique_ptr<Expr> object, std::unique_ptr<BlockExpr> body) :
                name{std::move(name)},
                object{std::move(object)},
                body{std::move(body)} {}

        ~ForExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitForExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitForExpr(*this);
        }
    };

    class GetExpr : public Expr {
    public:
        std::unique_ptr<Expr> object;
        Token name;
        Token oper;

        GetExpr(std::unique_ptr<Expr> object, Token name, Token oper) :
                object{std::move(object)},
                name{std::move(name)},
                oper{std::move(oper)} {}

        ~GetExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitGetExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitGetExpr(*this);
        }
    };

    class IfExpr : public Expr {
    public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockExpr> thenBody;
        std::unique_ptr<BlockExpr> elseBody;

        IfExpr(std::unique_ptr<Expr> condition,
                std::unique_ptr<BlockExpr> thenBody,
                std::unique_ptr<BlockExpr> elseBody) :
                condition{std::move(condition)},
                thenBody{std::move(thenBody)},
                elseBody{std::move(elseBody)} {}

        ~IfExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitIfExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitIfExpr(*this);
        }
    };

    class IntegerExpr : public Expr {
    public:
        int value;

        explicit IntegerExpr(int value) :
                value{value} {}

        ~IntegerExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitIntegerExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitIntegerExpr(*this);
        }
    };

    class LogicalExpr : public Expr {
    public:
        std::unique_ptr<Expr> left;
        std::unique_ptr<Expr> right;
        Token oper;

        LogicalExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, Token oper) :
                left{std::move(left)},
                right{std::move(right)},
                oper{std::move(oper)} {}

        ~LogicalExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitLogicalExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitLogicalExpr(*this);
        }
    };

    class StringExpr : public Expr {
    public:
        std::string value;

        explicit StringExpr(std::string value) :
                value{std::move(value)} {}

        ~StringExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitStringExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitStringExpr(*this);
        }
    };

    struct SwitchCase {
        std::unique_ptr<Pattern> pattern;
        std::unique_ptr<Expr> predicate;
        std::unique_ptr<BlockExpr> body;
    };

    class SwitchExpr : public Expr {
    public:
        std::unique_ptr<Expr> value;
        std::vector<SwitchCase> cases;

        SwitchExpr(std::unique_ptr<Expr> value, std::vector<SwitchCase>&& cases) :
                value{std::move(value)},
                cases{std::move(cases)} {}

        ~SwitchExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitSwitchExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitSwitchExpr(*this);
        }
    };

    class SymbolExpr : public Expr {
    public:
        Token name;

        explicit SymbolExpr(Token name) :
                name{std::move(name)} {}

        ~SymbolExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitSymbolExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitSymbolExpr(*this);
        }
    };

    class UnaryExpr : public Expr {
    public:
        std::unique_ptr<Expr> operand;
        Token oper;

        UnaryExpr(std::unique_ptr<Expr> operand, Token oper) :
                operand{std::move(operand)},
                oper{std::move(oper)} {}

        ~UnaryExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitUnaryExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitUnaryExpr(*this);
        }
    };

    class UnitExpr : public Expr {
    public:
        Token token;

        UnitExpr(Token token) :
                token{std::move(token)} {}

        ~UnitExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitUnitExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitUnitExpr(*this);
        }
    };

    class WhileExpr : public Expr {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockExpr> body;

        WhileExpr(std::unique_ptr<Expr> condition, std::unique_ptr<BlockExpr> body) :
                condition{std::move(condition)},
                body{std::move(body)} {}

        ~WhileExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitWhileExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitWhileExpr(*this);
        }
    };
}

#endif // ENACT_EXPR_H
