// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "Expr.h"

class Stmt {
public:
    class Block;
    class Expression;
    class Variable;

    template <class R>
    class Visitor {
    public:
        virtual R visitBlockStmt(Block None);
        virtual R visitExpressionStmt(Expression None);
        virtual R visitVariableStmt(Variable None);
    };

    virtual std::string accept(Stmt::Visitor<std::string> *visitor) = 0;
    virtual void accept(Stmt::Visitor<void> *visitor) = 0;
};

class Stmt::Block : public Stmt {
public:
    std::vector<std::shared_ptr<Stmt>> statements;

    Block(std::vector<std::shared_ptr<Stmt>> statements) : 
        statements{statements} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }
};

class Stmt::Expression : public Stmt {
public:
    std::shared_ptr<Expr> expr;

    Expression(std::shared_ptr<Expr> expr) : 
        expr{expr} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }
};

class Stmt::Variable : public Stmt {
public:
    Token name;
    std::string typeName;
    std::shared_ptr<Expr> initializer;
    bool isConst;

    Variable(Token name,std::string typeName,std::shared_ptr<Expr> initializer,bool isConst) : 
        name{name},typeName{typeName},initializer{initializer},isConst{isConst} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }
};

#endif // ENACT_STMT_H
