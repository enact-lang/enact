// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "Expr.h"
#include "../h/trivialStructs.h"

template <class R>
class StmtVisitor;

class Stmt {
public:
    virtual ~Stmt() = default;

    virtual std::string accept(StmtVisitor<std::string> *visitor) = 0;
    virtual void accept(StmtVisitor<void> *visitor) = 0;
};

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

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) :
            statements{std::move(statements)} {}
    ~BlockStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitBlockStmt(*this);
    }
};

class BreakStmt : public Stmt {
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

class ContinueStmt : public Stmt {
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

class EachStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> object;
    std::vector<std::unique_ptr<Stmt>> body;

    EachStmt(Token name, std::unique_ptr<Expr> object, std::vector<std::unique_ptr<Stmt>> body) :
            name{name},
            object{std::move(object)},
            body{std::move(body)} {}
    ~EachStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitEachStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitEachStmt(*this);
    }
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;

    ExpressionStmt(std::unique_ptr<Expr> expr) :
            expr{std::move(expr)} {}
    ~ExpressionStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitExpressionStmt(*this);
    }
};

class ForStmt : public Stmt {
public:
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::vector<std::unique_ptr<Stmt>> body;
    Token keyword;

    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition, std::unique_ptr<Expr> increment, std::vector<std::unique_ptr<Stmt>> body, Token keyword) :
            initializer{std::move(initializer)},
            condition{std::move(condition)},
            increment{std::move(increment)},
            body{std::move(body)},
            keyword{keyword} {}
    ~ForStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitForStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitForStmt(*this);
    }
};

class FunctionStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<const Typename> returnTypename;
    std::vector<Param> params;
    std::vector<std::unique_ptr<Stmt>> body;
    Type type;

    FunctionStmt(Token name, std::unique_ptr<const Typename> returnTypename, std::vector<Param>&& params, std::vector<std::unique_ptr<Stmt>> body, Type type) :
            name{name},
            returnTypename{std::move(returnTypename)},
            params{std::move(params)},
            body{std::move(body)},
            type{type} {}
    ~FunctionStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitFunctionStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitFunctionStmt(*this);
    }
};

class GivenStmt : public Stmt {
public:
    std::unique_ptr<Expr> value;
    std::vector<GivenCase> cases;

    GivenStmt(std::unique_ptr<Expr> value, std::vector<GivenCase>&& cases) :
            value{std::move(value)},
            cases{std::move(cases)} {}
    ~GivenStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitGivenStmt(*this);
    }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBlock;
    std::vector<std::unique_ptr<Stmt>> elseBlock;
    Token keyword;

    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> thenBlock, std::vector<std::unique_ptr<Stmt>> elseBlock, Token keyword) :
            condition{std::move(condition)},
            thenBlock{std::move(thenBlock)},
            elseBlock{std::move(elseBlock)},
            keyword{keyword} {}
    ~IfStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitIfStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitIfStmt(*this);
    }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;

    ReturnStmt(Token keyword, std::unique_ptr<Expr> value) :
            keyword{keyword},
            value{std::move(value)} {}
    ~ReturnStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitReturnStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitReturnStmt(*this);
    }
};

class StructStmt : public Stmt {
public:
    Token name;
    std::vector<Token> traits;
    std::vector<Field> fields;
    std::vector<std::unique_ptr<FunctionStmt>> methods;
    std::vector<std::unique_ptr<FunctionStmt>> assocFunctions;

    StructStmt(Token name, std::vector<Token> traits, std::vector<Field>&& fields, std::vector<std::unique_ptr<FunctionStmt>> methods, std::vector<std::unique_ptr<FunctionStmt>> assocFunctions) :
            name{name},
            traits{traits},
            fields{std::move(fields)},
            methods{std::move(methods)},
            assocFunctions{std::move(assocFunctions)} {}
    ~StructStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitStructStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitStructStmt(*this);
    }
};

class TraitStmt : public Stmt {
public:
    Token name;
    std::vector<std::unique_ptr<FunctionStmt>> methods;

    TraitStmt(Token name, std::vector<std::unique_ptr<FunctionStmt>> methods) :
            name{name},
            methods{std::move(methods)} {}
    ~TraitStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitTraitStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitTraitStmt(*this);
    }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
    Token keyword;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body, Token keyword) :
            condition{std::move(condition)},
            body{std::move(body)},
            keyword{keyword} {}
    ~WhileStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitWhileStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitWhileStmt(*this);
    }
};

class VariableStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<const Typename> typeName;
    std::unique_ptr<Expr> initializer;
    bool isConst;

    VariableStmt(Token name, std::unique_ptr<const Typename> typeName, std::unique_ptr<Expr> initializer, bool isConst) :
            name{name},
            typeName{std::move(typeName)},
            initializer{std::move(initializer)},
            isConst{isConst} {}
    ~VariableStmt() override = default;

    std::string accept(StmtVisitor<std::string> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }

    void accept(StmtVisitor<void> *visitor) override {
        return visitor->visitVariableStmt(*this);
    }
};

#endif // ENACT_STMT_H
