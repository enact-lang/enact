// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "Expr.h"
#include "../h/trivialStructs.h"

template <class R>
class StmtVisitor;

class StmtBase {
public:
    virtual ~StmtBase() = default;

    virtual std::string accept(StmtVisitor<std::string> *visitor) = 0;
    virtual void accept(StmtVisitor<void> *visitor) = 0;
};

typedef std::shared_ptr<StmtBase> Stmt;

class BlockStmt;
class BreakStmt;
class ContinueStmt;
class EachStmt;
class ExpressionStmt;
class ForStmt;
class FunctionStmt;
class GivenStmt;
class IfStmt;
class ReturnStmt;
class StructStmt;
class TraitStmt;
class WhileStmt;
class VariableStmt;

template <class R>
class StmtVisitor {
public:
    virtual R visitBlockStmt(BlockStmt& stmt) = 0;
    virtual R visitBreakStmt(BreakStmt& stmt) = 0;
    virtual R visitContinueStmt(ContinueStmt& stmt) = 0;
    virtual R visitEachStmt(EachStmt& stmt) = 0;
    virtual R visitExpressionStmt(ExpressionStmt& stmt) = 0;
    virtual R visitForStmt(ForStmt& stmt) = 0;
    virtual R visitFunctionStmt(FunctionStmt& stmt) = 0;
    virtual R visitGivenStmt(GivenStmt& stmt) = 0;
    virtual R visitIfStmt(IfStmt& stmt) = 0;
    virtual R visitReturnStmt(ReturnStmt& stmt) = 0;
    virtual R visitStructStmt(StructStmt& stmt) = 0;
    virtual R visitTraitStmt(TraitStmt& stmt) = 0;
    virtual R visitWhileStmt(WhileStmt& stmt) = 0;
    virtual R visitVariableStmt(VariableStmt& stmt) = 0;
};

class BlockStmt : public StmtBase {
public:
    std::vector<Stmt> statements;

    BlockStmt(std::vector<Stmt> statements) : 
        statements{statements} {}
    ~BlockStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }
};

class BreakStmt : public StmtBase {
public:
    Token keyword;

    BreakStmt(Token keyword) : 
        keyword{keyword} {}
    ~BreakStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitBreakStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitBreakStmt(*this);
    }
};

class ContinueStmt : public StmtBase {
public:
    Token keyword;

    ContinueStmt(Token keyword) : 
        keyword{keyword} {}
    ~ContinueStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitContinueStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitContinueStmt(*this);
    }
};

class EachStmt : public StmtBase {
public:
    Token name;
    Expr object;
    std::vector<Stmt> body;

    EachStmt(Token name,Expr object,std::vector<Stmt> body) : 
        name{name},object{object},body{body} {}
    ~EachStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitEachStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitEachStmt(*this);
    }
};

class ExpressionStmt : public StmtBase {
public:
    Expr expr;

    ExpressionStmt(Expr expr) : 
        expr{expr} {}
    ~ExpressionStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }
};

class ForStmt : public StmtBase {
public:
    Stmt initializer;
    Expr condition;
    Expr increment;
    std::vector<Stmt> body;
    Token keyword;

    ForStmt(Stmt initializer,Expr condition,Expr increment,std::vector<Stmt> body,Token keyword) : 
        initializer{initializer},condition{condition},increment{increment},body{body},keyword{keyword} {}
    ~ForStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitForStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitForStmt(*this);
    }
};

class FunctionStmt : public StmtBase {
public:
    Token name;
    std::string returnTypeName;
    std::vector<NamedTypename> params;
    std::vector<Stmt> body;
    Type type;

    FunctionStmt(Token name,std::string returnTypeName,std::vector<NamedTypename> params,std::vector<Stmt> body,Type type) : 
        name{name},returnTypeName{returnTypeName},params{params},body{body},type{type} {}
    ~FunctionStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitFunctionStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitFunctionStmt(*this);
    }
};

class GivenStmt : public StmtBase {
public:
    Expr value;
    std::vector<GivenCase> cases;

    GivenStmt(Expr value,std::vector<GivenCase> cases) : 
        value{value},cases{cases} {}
    ~GivenStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }
};

class IfStmt : public StmtBase {
public:
    Expr condition;
    std::vector<Stmt> thenBlock;
    std::vector<Stmt> elseBlock;
    Token keyword;

    IfStmt(Expr condition,std::vector<Stmt> thenBlock,std::vector<Stmt> elseBlock,Token keyword) : 
        condition{condition},thenBlock{thenBlock},elseBlock{elseBlock},keyword{keyword} {}
    ~IfStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitIfStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitIfStmt(*this);
    }
};

class ReturnStmt : public StmtBase {
public:
    Token keyword;
    Expr value;

    ReturnStmt(Token keyword,Expr value) : 
        keyword{keyword},value{value} {}
    ~ReturnStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitReturnStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitReturnStmt(*this);
    }
};

class StructStmt : public StmtBase {
public:
    Token name;
    std::vector<Token> traits;
    std::vector<NamedTypename> fields;
    std::vector<std::shared_ptr<FunctionStmt>> methods;
    std::vector<std::shared_ptr<FunctionStmt>> assocFunctions;

    StructStmt(Token name,std::vector<Token> traits,std::vector<NamedTypename> fields,std::vector<std::shared_ptr<FunctionStmt>> methods,std::vector<std::shared_ptr<FunctionStmt>> assocFunctions) : 
        name{name},traits{traits},fields{fields},methods{methods},assocFunctions{assocFunctions} {}
    ~StructStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitStructStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitStructStmt(*this);
    }
};

class TraitStmt : public StmtBase {
public:
    Token name;
    std::vector<std::shared_ptr<FunctionStmt>> methods;

    TraitStmt(Token name,std::vector<std::shared_ptr<FunctionStmt>> methods) : 
        name{name},methods{methods} {}
    ~TraitStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitTraitStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitTraitStmt(*this);
    }
};

class WhileStmt : public StmtBase {
public:
    Expr condition;
    std::vector<Stmt> body;
    Token keyword;

    WhileStmt(Expr condition,std::vector<Stmt> body,Token keyword) : 
        condition{condition},body{body},keyword{keyword} {}
    ~WhileStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitWhileStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitWhileStmt(*this);
    }
};

class VariableStmt : public StmtBase {
public:
    Token name;
    std::string typeName;
    Expr initializer;
    bool isConst;

    VariableStmt(Token name,std::string typeName,Expr initializer,bool isConst) : 
        name{name},typeName{typeName},initializer{initializer},isConst{isConst} {}
    ~VariableStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }
};

#endif // ENACT_STMT_H
