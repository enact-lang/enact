// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include "../h/Token.h"
#include <memory>
#include <vector>

class Expr {
public:
    class Array;
    class Assign;
    class Binary;
    class Boolean;
    class Call;
    class Field;
    class Logical;
    class Nil;
    class Number;
    class Reference;
    class String;
    class Subscript;
    class Ternary;
    class Unary;
    class Any;
    class Variable;

    template <class R>
    class Visitor {
    public:
        virtual R visitArrayExpr(Array None);
        virtual R visitAssignExpr(Assign None);
        virtual R visitBinaryExpr(Binary None);
        virtual R visitBooleanExpr(Boolean None);
        virtual R visitCallExpr(Call None);
        virtual R visitFieldExpr(Field None);
        virtual R visitLogicalExpr(Logical None);
        virtual R visitNilExpr(Nil None);
        virtual R visitNumberExpr(Number None);
        virtual R visitReferenceExpr(Reference None);
        virtual R visitStringExpr(String None);
        virtual R visitSubscriptExpr(Subscript None);
        virtual R visitTernaryExpr(Ternary None);
        virtual R visitUnaryExpr(Unary None);
        virtual R visitAnyExpr(Any None);
        virtual R visitVariableExpr(Variable None);
    };

    virtual std::string accept(Expr::Visitor<std::string> *visitor) = 0;
    virtual void accept(Expr::Visitor<void> *visitor) = 0;
};

class Expr::Array : public Expr {
public:
    std::vector<std::shared_ptr<Expr>> value;

    Array(std::vector<std::shared_ptr<Expr>> value) : 
        value{value} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitArrayExpr(*this);
    }
};

class Expr::Assign : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
    Token oper;

    Assign(std::shared_ptr<Expr> left,std::shared_ptr<Expr> right,Token oper) : 
        left{left},right{right},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitAssignExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitAssignExpr(*this);
    }
};

class Expr::Binary : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
    Token oper;

    Binary(std::shared_ptr<Expr> left,std::shared_ptr<Expr> right,Token oper) : 
        left{left},right{right},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitBinaryExpr(*this);
    }
};

class Expr::Boolean : public Expr {
public:
    bool value;

    Boolean(bool value) : 
        value{value} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitBooleanExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitBooleanExpr(*this);
    }
};

class Expr::Call : public Expr {
public:
    std::shared_ptr<Expr> callee;
    std::vector<std::shared_ptr<Expr>> arguments;
    Token paren;

    Call(std::shared_ptr<Expr> callee,std::vector<std::shared_ptr<Expr>> arguments,Token paren) : 
        callee{callee},arguments{arguments},paren{paren} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitCallExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitCallExpr(*this);
    }
};

class Expr::Field : public Expr {
public:
    std::shared_ptr<Expr> object;
    Token name;
    Token oper;

    Field(std::shared_ptr<Expr> object,Token name,Token oper) : 
        object{object},name{name},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitFieldExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitFieldExpr(*this);
    }
};

class Expr::Logical : public Expr {
public:
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
    Token oper;

    Logical(std::shared_ptr<Expr> left,std::shared_ptr<Expr> right,Token oper) : 
        left{left},right{right},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitLogicalExpr(*this);
    }
};

class Expr::Nil : public Expr {
public:
    Nil() = default;

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitNilExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitNilExpr(*this);
    }
};

class Expr::Number : public Expr {
public:
    double value;

    Number(double value) : 
        value{value} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitNumberExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitNumberExpr(*this);
    }
};

class Expr::Reference : public Expr {
public:
    std::shared_ptr<Expr> object;
    Token oper;

    Reference(std::shared_ptr<Expr> object,Token oper) : 
        object{object},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitReferenceExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitReferenceExpr(*this);
    }
};

class Expr::String : public Expr {
public:
    std::string value;

    String(std::string value) : 
        value{value} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitStringExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitStringExpr(*this);
    }
};

class Expr::Subscript : public Expr {
public:
    std::shared_ptr<Expr> object;
    std::shared_ptr<Expr> index;
    Token square;

    Subscript(std::shared_ptr<Expr> object,std::shared_ptr<Expr> index,Token square) : 
        object{object},index{index},square{square} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitSubscriptExpr(*this);
    }
};

class Expr::Ternary : public Expr {
public:
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Expr> thenExpr;
    std::shared_ptr<Expr> elseExpr;
    Token oper;

    Ternary(std::shared_ptr<Expr> condition,std::shared_ptr<Expr> thenExpr,std::shared_ptr<Expr> elseExpr,Token oper) : 
        condition{condition},thenExpr{thenExpr},elseExpr{elseExpr},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitTernaryExpr(*this);
    }
};

class Expr::Unary : public Expr {
public:
    std::shared_ptr<Expr> operand;
    Token oper;

    Unary(std::shared_ptr<Expr> operand,Token oper) : 
        operand{operand},oper{oper} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitUnaryExpr(*this);
    }
};

class Expr::Any : public Expr {
public:
    Any() = default;

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitAnyExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitAnyExpr(*this);
    }
};

class Expr::Variable : public Expr {
public:
    Token name;

    Variable(Token name) : 
        name{name} {}

    std::string accept(Expr::Visitor<std::string> *visitor) override {
        return visitor->visitVariableExpr(*this);
    }

    void accept(Expr::Visitor<void> *visitor) override {
        return visitor->visitVariableExpr(*this);
    }
};

#endif // ENACT_EXPR_H
