// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include "../h/Token.h"
#include <memory>
#include <vector>

class Expr {
public:
    class Assign;
    class Binary;
    class Boolean;
    class Call;
    class Logical;
    class Nil;
    class Number;
    class String;
    class Ternary;
    class Unary;
    class Variable;

    template <class R>
    class Visitor {
    public:
        virtual R visitAssignExpr(Assign expr);
        virtual R visitBinaryExpr(Binary expr);
        virtual R visitBooleanExpr(Boolean expr);
        virtual R visitCallExpr(Call expr);
        virtual R visitLogicalExpr(Logical expr);
        virtual R visitNilExpr(Nil expr);
        virtual R visitNumberExpr(Number expr);
        virtual R visitStringExpr(String expr);
        virtual R visitTernaryExpr(Ternary expr);
        virtual R visitUnaryExpr(Unary expr);
        virtual R visitVariableExpr(Variable expr);
    };

    virtual std::string accept(Expr::Visitor<std::string> *visitor) = 0;
    virtual void accept(Expr::Visitor<void> *visitor) = 0;
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
    Nil() {}

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
