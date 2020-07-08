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
    class CastExpr;
    class FloatExpr;
    class ForExpr;
    class FieldExpr;
    class IfExpr;
    class IntegerExpr;
    class InterpolationExpr;
    class LogicalExpr;
    class SpecificationExpr;
    class StringExpr;
    class SwitchExpr;
    class SymbolExpr;
    class TupleExpr;
    class UnaryExpr;
    class UnitExpr;
    class WhileExpr;

    template<class R>
    class ExprVisitor {
    public:
        R visitExpr(Expr& expr) {
            return expr.accept(this);
        }

        virtual R visitAssignExpr(AssignExpr& expr) = 0;
        virtual R visitBinaryExpr(BinaryExpr& expr) = 0;
        virtual R visitBlockExpr(BlockExpr& expr) = 0;
        virtual R visitBooleanExpr(BooleanExpr& expr) = 0;
        virtual R visitCallExpr(CallExpr& expr) = 0;
        virtual R visitCastExpr(CastExpr& expr) = 0;
        virtual R visitFloatExpr(FloatExpr& expr) = 0;
        virtual R visitForExpr(ForExpr& expr) = 0;
        virtual R visitGetExpr(FieldExpr& expr) = 0;
        virtual R visitIfExpr(IfExpr& expr) = 0;
        virtual R visitIntegerExpr(IntegerExpr& expr) = 0;
        virtual R visitInterpolationExpr(InterpolationExpr& expr) = 0;
        virtual R visitLogicalExpr(LogicalExpr& expr) = 0;
        virtual R visitSpecificationExpr(SpecificationExpr& expr) = 0;
        virtual R visitStringExpr(StringExpr& expr) = 0;
        virtual R visitSwitchExpr(SwitchExpr& expr) = 0;
        virtual R visitSymbolExpr(SymbolExpr& expr) = 0;
        virtual R visitTupleExpr(TupleExpr& expr) = 0;
        virtual R visitUnaryExpr(UnaryExpr& expr) = 0;
        virtual R visitUnitExpr(UnitExpr& expr) = 0;
        virtual R visitWhileExpr(WhileExpr& expr) = 0;
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
        std::vector<std::unique_ptr<Expr>> args;
        Token paren;

        CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args, Token paren) :
                callee{std::move(callee)},
                args{std::move(args)},
                paren{std::move(paren)} {}

        ~CallExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitCallExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitCallExpr(*this);
        }
    };

    class CastExpr : public Expr {
    public:
    public:
        std::unique_ptr<Expr> expr;
        std::unique_ptr<const Typename> typename_;
        Token oper;

        CastExpr(std::unique_ptr<Expr> expr, std::unique_ptr<const Typename> typename_, Token oper) :
                expr{std::move(expr)},
                typename_{std::move(typename_)},
                oper{std::move(oper)} {}

        ~CastExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitCastExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitCastExpr(*this);
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

    class FieldExpr : public Expr {
    public:
        std::unique_ptr<Expr> object;
        Token name;
        Token oper;

        FieldExpr(std::unique_ptr<Expr> object, Token name, Token oper) :
                object{std::move(object)},
                name{std::move(name)},
                oper{std::move(oper)} {}

        ~FieldExpr() override = default;

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
        Token keyword;

        IfExpr(std::unique_ptr<Expr> condition,
               std::unique_ptr<BlockExpr> thenBody,
               std::unique_ptr<BlockExpr> elseBody,
               Token keyword) :
                condition{std::move(condition)},
                thenBody{std::move(thenBody)},
                elseBody{std::move(elseBody)},
                keyword{std::move(keyword)} {}

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

    class InterpolationExpr : public Expr {
    public:
        std::unique_ptr<StringExpr> start;
        std::unique_ptr<Expr> interpolated;
        std::unique_ptr<Expr> end;
        Token token;

        InterpolationExpr(
                std::unique_ptr<StringExpr> start,
                std::unique_ptr<Expr> interpolated,
                std::unique_ptr<Expr> end,
                Token token) :
                start{std::move(start)},
                interpolated{std::move(interpolated)},
                end{std::move(end)},
                token{std::move(token)} {}

        ~InterpolationExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitInterpolationExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitInterpolationExpr(*this);
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

    class SpecificationExpr : public Expr {
    public:
        struct TypeArg {
            TypeArg(std::unique_ptr<const VariableTypename> name, std::unique_ptr<const Typename> value) :
                    name{std::move(name)},
                    value{std::move(value)} {
            }

            std::unique_ptr<const VariableTypename> name;
            std::unique_ptr<const Typename> value;
        };

        std::unique_ptr<Expr> expr;
        std::vector<TypeArg> args;
        Token square;

        SpecificationExpr(std::unique_ptr<Expr> expr, std::vector<TypeArg> args, Token square) :
                expr{std::move(expr)},
                args{std::move(args)},
                square{std::move(square)} {}

        ~SpecificationExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitSpecificationExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitSpecificationExpr(*this);
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
        Token keyword;
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

    class TupleExpr : public Expr {
    public:
        std::vector<std::unique_ptr<Expr>> elems;
        Token paren;

        TupleExpr(std::vector<std::unique_ptr<Expr>> elems, Token paren) :
                elems{std::move(elems)},
                paren{std::move(paren)} {}

        ~TupleExpr() override = default;

        std::string accept(ExprVisitor<std::string> *visitor) override {
            return visitor->visitTupleExpr(*this);
        }

        void accept(ExprVisitor<void> *visitor) override {
            return visitor->visitTupleExpr(*this);
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
    public:
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockExpr> body;
        Token keyword;

        WhileExpr(std::unique_ptr<Expr> condition, std::unique_ptr<BlockExpr> body, Token keyword) :
                condition{std::move(condition)},
                body{std::move(body)},
                keyword{std::move(keyword)} {}

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
