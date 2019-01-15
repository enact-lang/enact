// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "Expr.h"
#include "../h/GivenCase.h"

class Stmt {
public:
    class Block;
    class Each;
    class Expression;
    class For;
    class Given;
    class If;
    class While;
    class Variable;

    template <class R>
    class Visitor {
    public:
        virtual R visitBlockStmt(Block None);
        virtual R visitEachStmt(Each None);
        virtual R visitExpressionStmt(Expression None);
        virtual R visitForStmt(For None);
        virtual R visitGivenStmt(Given None);
        virtual R visitIfStmt(If None);
        virtual R visitWhileStmt(While None);
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

class Stmt::Each : public Stmt {
public:
    Token name;
    std::shared_ptr<Expr> object;
    std::vector<std::shared_ptr<Stmt>> body;

    Each(Token name,std::shared_ptr<Expr> object,std::vector<std::shared_ptr<Stmt>> body) : 
        name{name},object{object},body{body} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitEachStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitEachStmt(*this);
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

class Stmt::For : public Stmt {
public:
    std::shared_ptr<Stmt> initializer;
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Expr> increment;
    std::vector<std::shared_ptr<Stmt>> body;

    For(std::shared_ptr<Stmt> initializer,std::shared_ptr<Expr> condition,std::shared_ptr<Expr> increment,std::vector<std::shared_ptr<Stmt>> body) : 
        initializer{initializer},condition{condition},increment{increment},body{body} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitForStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitForStmt(*this);
    }
};

class Stmt::Given : public Stmt {
public:
    std::shared_ptr<Expr> value;
    std::vector<GivenCase> cases;

    Given(std::shared_ptr<Expr> value,std::vector<GivenCase> cases) : 
        value{value},cases{cases} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }
};

class Stmt::If : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> thenBlock;
    std::vector<std::shared_ptr<Stmt>> elseBlock;

    If(std::shared_ptr<Expr> condition,std::vector<std::shared_ptr<Stmt>> thenBlock,std::vector<std::shared_ptr<Stmt>> elseBlock) : 
        condition{condition},thenBlock{thenBlock},elseBlock{elseBlock} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitIfStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitIfStmt(*this);
    }
};

class Stmt::While : public Stmt {
public:
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;

    While(std::shared_ptr<Expr> condition,std::vector<std::shared_ptr<Stmt>> body) : 
        condition{condition},body{body} {}

    std::string accept(Stmt::Visitor<std::string> *visitor) override {
        return visitor->visitWhileStmt(*this);
    }

    void accept(Stmt::Visitor<void> *visitor) override {
        return visitor->visitWhileStmt(*this);
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
