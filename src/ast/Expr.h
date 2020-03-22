// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include "../h/Type.h"
#include "../h/Typename.h"
#include <memory>
#include <vector>

template <class R>
class ExprVisitor;

class Expr {
    Type m_type = nullptr;
public:
    virtual void setType(Type t) { m_type = t; }
    virtual const Type& getType() {
        ENACT_ASSERT(m_type != nullptr, "Expr::getType(): Tried to get uninitialized type.");
        return m_type;
    }
    virtual ~Expr() = default;

    virtual std::string accept(ExprVisitor<std::string> *visitor) = 0;
    virtual void accept(ExprVisitor<void> *visitor) = 0;
};

class AllotExpr;
class AnyExpr;
class ArrayExpr;
class AssignExpr;
class BinaryExpr;
class BooleanExpr;
class CallExpr;
class FloatExpr;
class GetExpr;
class IntegerExpr;
class LogicalExpr;
class NilExpr;
class StringExpr;
class SubscriptExpr;
class TernaryExpr;
class UnaryExpr;
class VariableExpr;

template <class R>
class ExprVisitor {
public:
    virtual R visitAllotExpr(AllotExpr& expr) = 0;
    virtual R visitAnyExpr(AnyExpr& expr) = 0;
    virtual R visitArrayExpr(ArrayExpr& expr) = 0;
    virtual R visitAssignExpr(AssignExpr& expr) = 0;
    virtual R visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual R visitBooleanExpr(BooleanExpr& expr) = 0;
    virtual R visitCallExpr(CallExpr& expr) = 0;
    virtual R visitFloatExpr(FloatExpr& expr) = 0;
    virtual R visitGetExpr(GetExpr& expr) = 0;
    virtual R visitIntegerExpr(IntegerExpr& expr) = 0;
    virtual R visitLogicalExpr(LogicalExpr& expr) = 0;
    virtual R visitNilExpr(NilExpr& expr) = 0;
    virtual R visitStringExpr(StringExpr& expr) = 0;
    virtual R visitSubscriptExpr(SubscriptExpr& expr) = 0;
    virtual R visitTernaryExpr(TernaryExpr& expr) = 0;
    virtual R visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual R visitVariableExpr(VariableExpr& expr) = 0;
};

class AllotExpr : public Expr {
public:
    std::unique_ptr<SubscriptExpr> target;
    std::unique_ptr<Expr> value;
    Token oper;

    AllotExpr(std::unique_ptr<SubscriptExpr> target,std::unique_ptr<Expr> value,Token oper) :
            target{std::move(target)},
            value{std::move(value)},
            oper{oper} {}
    ~AllotExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitAllotExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitAllotExpr(*this);
    }
};

class AnyExpr : public Expr {
public:
    AnyExpr() = default;
    ~AnyExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitAnyExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitAnyExpr(*this);
    }
};

class ArrayExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> value;
    Token square;
    std::unique_ptr<const Typename> typeName;

    ArrayExpr(std::vector<std::unique_ptr<Expr>> value,Token square,std::unique_ptr<const Typename> typeName) :
            value{std::move(value)},
            square{square},
            typeName{std::move(typeName)} {}
    ~ArrayExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }
};

class AssignExpr : public Expr {
public:
    std::unique_ptr<VariableExpr> target;
    std::unique_ptr<Expr> value;
    Token oper;

    AssignExpr(std::unique_ptr<VariableExpr> target,std::unique_ptr<Expr> value,Token oper) :
            target{std::move(target)},
            value{std::move(value)},
            oper{oper} {}
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

    BinaryExpr(std::unique_ptr<Expr> left,std::unique_ptr<Expr> right,Token oper) :
            left{std::move(left)},
            right{std::move(right)},
            oper{oper} {}
    ~BinaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }
};

class BooleanExpr : public Expr {
public:
    bool value;

    BooleanExpr(bool value) : 
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

    CallExpr(std::unique_ptr<Expr> callee,std::vector<std::unique_ptr<Expr>> arguments,Token paren) :
            callee{std::move(callee)},
            arguments{std::move(arguments)},
            paren{paren} {}
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

    FloatExpr(double value) : 
            value{value} {}
    ~FloatExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitFloatExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitFloatExpr(*this);
    }
};

class GetExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token name;
    Token oper;

    GetExpr(std::unique_ptr<Expr> object,Token name,Token oper) :
            object{std::move(object)},
            name{name},
            oper{oper} {}
    ~GetExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitGetExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitGetExpr(*this);
    }
};

class IntegerExpr : public Expr {
public:
    int value;

    IntegerExpr(int value) : 
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

    LogicalExpr(std::unique_ptr<Expr> left,std::unique_ptr<Expr> right,Token oper) :
        left{std::move(left)},
        right{std::move(right)},
        oper{oper} {}
    ~LogicalExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }
};

class NilExpr : public Expr {
public:
    NilExpr() = default;
    ~NilExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitNilExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitNilExpr(*this);
    }
};

class StringExpr : public Expr {
public:
    std::string value;

    StringExpr(std::string value) : 
            value{value} {}
    ~StringExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitStringExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitStringExpr(*this);
    }
};

class SubscriptExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    Token square;

    SubscriptExpr(std::unique_ptr<Expr> object,std::unique_ptr<Expr> index,Token square) :
            object{std::move(object)},
            index{std::move(index)},
            square{square} {}
    ~SubscriptExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }
};

class TernaryExpr : public Expr {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;
    Token oper;

    TernaryExpr(std::unique_ptr<Expr> condition,std::unique_ptr<Expr> thenExpr,std::unique_ptr<Expr> elseExpr,Token oper) :
            condition{std::move(condition)},
            thenExpr{std::move(thenExpr)},
            elseExpr{std::move(elseExpr)},
            oper{oper} {}
    ~TernaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }
};

class UnaryExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;
    Token oper;

    UnaryExpr(std::unique_ptr<Expr> operand,Token oper) :
            operand{std::move(operand)},
            oper{oper} {}
    ~UnaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }
};

class VariableExpr : public Expr {
public:
    Token name;

    VariableExpr(Token name) : 
            name{name} {}
    ~VariableExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitVariableExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitVariableExpr(*this);
    }
};

#endif // ENACT_EXPR_H
