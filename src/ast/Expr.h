#ifndef ENACT_EXPR_H
#define ENACT_EXPR_H

#include "../h/Token.h"
#include "../h/Value.h"

#include <memory>
#include <vector>

class Expr {
public:
    class Assign;
    class Binary;
    class Boolean;
    class Call;
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
        virtual R visitNilExpr(Nil expr);
        virtual R visitNumberExpr(Number expr);
        virtual R visitStringExpr(String expr);
        virtual R visitTernaryExpr(Ternary expr);
        virtual R visitUnaryExpr(Unary expr);
        virtual R visitVariableExpr(Variable expr);
    };

    virtual void accept(Expr::Visitor<void> *visitor) = 0;
    virtual std::string accept(Expr::Visitor<std::string> *visitor) = 0;
};

#define EXPR_ACCEPT_FUNCTION(T, name) \
    T accept(Expr::Visitor<T> *visitor) override { \
        return visitor->name(*this); \
    }

class Expr::Assign : public Expr {
public:
    Sp<Expr> left, right;
    Token oper;

    Assign(Sp<Expr> left, Sp<Expr> right, Token oper) : left{left}, right{right}, oper{std::move(oper)} {}

    EXPR_ACCEPT_FUNCTION(void, visitAssignExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitAssignExpr);
};

class Expr::Binary : public Expr {
public:
    Sp<Expr> left, right;
    Token oper;

    Binary(Sp<Expr> left, Sp<Expr> right, Token oper) : left{left}, right{right}, oper{std::move(oper)} {}

    EXPR_ACCEPT_FUNCTION(void, visitBinaryExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitBinaryExpr);
};

class Expr::Boolean : public Expr {
public:
    bool value;

    Boolean(bool value) : value{value} {}

    EXPR_ACCEPT_FUNCTION(void, visitBooleanExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitBooleanExpr);
};

class Expr::Call : public Expr {
public:
    Sp<Expr> callee;
    std::vector<Sp<Expr>> arguments;
    Token paren;

    Call(Sp<Expr> callee, std::vector<Sp<Expr>> arguments, Token paren) : callee{callee}, arguments{std::move(arguments)}, paren{std::move(paren)} {}

    EXPR_ACCEPT_FUNCTION(void, visitCallExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitCallExpr);
};

class Expr::Nil : public Expr {
public:
    Nil() = default;

    EXPR_ACCEPT_FUNCTION(void, visitNilExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitNilExpr);
};

class Expr::Number : public Expr {
public:
    double value;

    Number(double value) : value{value} {}

    EXPR_ACCEPT_FUNCTION(void, visitNumberExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitNumberExpr);
};

class Expr::String : public Expr {
public:
    std::string value;

    String(std::string value) : value{std::move(value)} {}

    EXPR_ACCEPT_FUNCTION(void, visitStringExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitStringExpr);
};


class Expr::Ternary : public Expr {
public:
    Sp<Expr> condition;
    Sp<Expr> thenExpr;
    Sp<Expr> elseExpr;
    Token oper;

    Ternary(Sp<Expr> condition, Sp<Expr> thenExpr, Sp<Expr> elseExpr, Token oper) : condition{condition}, thenExpr{thenExpr}, elseExpr{elseExpr}, oper{std::move(oper)} {}

    EXPR_ACCEPT_FUNCTION(void, visitTernaryExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitTernaryExpr);
};

class Expr::Unary : public Expr {
public:
    Sp<Expr> operand;
    Token oper;

    Unary(Sp<Expr> operand, Token oper) : operand{operand}, oper{std::move(oper)} {}

    EXPR_ACCEPT_FUNCTION(void, visitUnaryExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitUnaryExpr);
};

class Expr::Variable : public Expr {
public:
    Token name;

    explicit Variable(Token name) : name{std::move(name)} {}

    EXPR_ACCEPT_FUNCTION(void, visitVariableExpr);
    EXPR_ACCEPT_FUNCTION(std::string, visitVariableExpr);
};

#undef EXPR_ACCEPT_FUNCTION

#endif //ENACT_EXPR_H
