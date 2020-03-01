// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include "../h/Token.h"
#include "../h/Type.h"
#include <memory>
#include <vector>

template <class R>
class ExprVisitor;

class ExprBase {
    Type m_type = nullptr;
    bool m_isVar = false;
    bool m_isLvalue = false;
public:
    virtual void setType(Type t) { m_type = t; }
    virtual const Type& getType() {
        ENACT_ASSERT(m_type != nullptr, "Expr::getType(): Tried to get uninitialized type.");
        return m_type;
    }
    virtual void markVar() { m_isVar = true; }
    virtual bool isVar() const { return m_isVar; }
    virtual void markLvalue() { m_isLvalue = true; }
    virtual bool isLvalue() const { return m_isLvalue; }
    virtual ~ExprBase() = default;

    virtual std::string accept(ExprVisitor<std::string> *visitor) = 0;
    virtual void accept(ExprVisitor<void> *visitor) = 0;
};

typedef std::shared_ptr<ExprBase> Expr;

class AnyExpr;
class ArrayExpr;
class AssignExpr;
class BinaryExpr;
class BooleanExpr;
class CallExpr;
class FieldExpr;
class FloatExpr;
class IntegerExpr;
class LogicalExpr;
class NilExpr;
class ReferenceExpr;
class StringExpr;
class SubscriptExpr;
class TernaryExpr;
class UnaryExpr;
class VariableExpr;

template <class R>
class ExprVisitor {
public:
    virtual R visitAnyExpr(AnyExpr& expr) = 0;
    virtual R visitArrayExpr(ArrayExpr& expr) = 0;
    virtual R visitAssignExpr(AssignExpr& expr) = 0;
    virtual R visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual R visitBooleanExpr(BooleanExpr& expr) = 0;
    virtual R visitCallExpr(CallExpr& expr) = 0;
    virtual R visitFieldExpr(FieldExpr& expr) = 0;
    virtual R visitFloatExpr(FloatExpr& expr) = 0;
    virtual R visitIntegerExpr(IntegerExpr& expr) = 0;
    virtual R visitLogicalExpr(LogicalExpr& expr) = 0;
    virtual R visitNilExpr(NilExpr& expr) = 0;
    virtual R visitReferenceExpr(ReferenceExpr& expr) = 0;
    virtual R visitStringExpr(StringExpr& expr) = 0;
    virtual R visitSubscriptExpr(SubscriptExpr& expr) = 0;
    virtual R visitTernaryExpr(TernaryExpr& expr) = 0;
    virtual R visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual R visitVariableExpr(VariableExpr& expr) = 0;
};

class AnyExpr : public ExprBase {
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

class ArrayExpr : public ExprBase {
public:
    std::vector<Expr> value;
    Token square;
    std::string typeName;

    ArrayExpr(std::vector<Expr> value,Token square,std::string typeName) : 
        value{value},square{square},typeName{typeName} {}
    ~ArrayExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }
};

class AssignExpr : public ExprBase {
public:
    Expr left;
    Expr right;
    Token oper;

    AssignExpr(Expr left,Expr right,Token oper) : 
        left{left},right{right},oper{oper} {}
    ~AssignExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitAssignExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitAssignExpr(*this);
    }
};

class BinaryExpr : public ExprBase {
public:
    Expr left;
    Expr right;
    Token oper;

    BinaryExpr(Expr left,Expr right,Token oper) : 
        left{left},right{right},oper{oper} {}
    ~BinaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }
};

class BooleanExpr : public ExprBase {
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

class CallExpr : public ExprBase {
public:
    Expr callee;
    std::vector<Expr> arguments;
    Token paren;

    CallExpr(Expr callee,std::vector<Expr> arguments,Token paren) : 
        callee{callee},arguments{arguments},paren{paren} {}
    ~CallExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitCallExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitCallExpr(*this);
    }
};

class FieldExpr : public ExprBase {
public:
    Expr object;
    Token name;
    Token oper;

    FieldExpr(Expr object,Token name,Token oper) : 
        object{object},name{name},oper{oper} {}
    ~FieldExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitFieldExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitFieldExpr(*this);
    }
};

class FloatExpr : public ExprBase {
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

class IntegerExpr : public ExprBase {
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

class LogicalExpr : public ExprBase {
public:
    Expr left;
    Expr right;
    Token oper;

    LogicalExpr(Expr left,Expr right,Token oper) : 
        left{left},right{right},oper{oper} {}
    ~LogicalExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }
};

class NilExpr : public ExprBase {
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

class ReferenceExpr : public ExprBase {
public:
    Expr object;
    Token oper;
    bool isVar;

    ReferenceExpr(Expr object,Token oper,bool isVar) : 
        object{object},oper{oper},isVar{isVar} {}
    ~ReferenceExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitReferenceExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitReferenceExpr(*this);
    }
};

class StringExpr : public ExprBase {
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

class SubscriptExpr : public ExprBase {
public:
    Expr object;
    Expr index;
    Token square;

    SubscriptExpr(Expr object,Expr index,Token square) : 
        object{object},index{index},square{square} {}
    ~SubscriptExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }
};

class TernaryExpr : public ExprBase {
public:
    Expr condition;
    Expr thenExpr;
    Expr elseExpr;
    Token oper;

    TernaryExpr(Expr condition,Expr thenExpr,Expr elseExpr,Token oper) : 
        condition{condition},thenExpr{thenExpr},elseExpr{elseExpr},oper{oper} {}
    ~TernaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }
};

class UnaryExpr : public ExprBase {
public:
    Expr operand;
    Token oper;

    UnaryExpr(Expr operand,Token oper) : 
        operand{operand},oper{oper} {}
    ~UnaryExpr() override = default;

    std::string accept(ExprVisitor<std::string> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }

    void accept(ExprVisitor<void> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }
};

class VariableExpr : public ExprBase {
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
