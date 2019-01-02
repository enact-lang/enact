#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "Expr.h"

class Stmt {
public:
    class Expression;
    class Print;
    class Variable;

    template <class R>
    class Visitor {
    public:
        virtual R visitExpressionStmt(Expression stmt);
        virtual R visitPrintStmt(Print stmt);
        virtual R visitVariableStmt(Variable stmt);
    };

    virtual void accept(Stmt::Visitor<void> *visitor) = 0;
    virtual std::string accept(Stmt::Visitor<std::string> *visitor) = 0;
};

#define STMT_ACCEPT_FUNCTION(T, name) \
    T accept(Stmt::Visitor<T> *visitor) override { \
        return visitor->name(*this); \
    }

class Stmt::Expression : public Stmt {
public:
    Sp<Expr> expr;

    Expression(Sp<Expr> expr) : expr{expr} {}

    STMT_ACCEPT_FUNCTION(void, visitExpressionStmt);
    STMT_ACCEPT_FUNCTION(std::string, visitExpressionStmt);
};

class Stmt::Print : public Stmt {
public:
    Sp<Expr> expr;

    Print(Sp<Expr> expr) : expr{expr} {}

    STMT_ACCEPT_FUNCTION(void, visitPrintStmt);
    STMT_ACCEPT_FUNCTION(std::string, visitPrintStmt);
};

class Stmt::Variable : public Stmt {
public:
    Token name;
    Sp<Expr> initializer;
    bool isConst;

    Variable(Token name, Sp<Expr> initializer, bool isConst) : name{std::move(name)}, initializer{initializer}, isConst{isConst} {}

    STMT_ACCEPT_FUNCTION(void, visitVariableStmt);
    STMT_ACCEPT_FUNCTION(std::string, visitVariableStmt);
};

#undef STMT_ACCEPT_FUNCTION

#endif //ENACT_STMT_H
