// This file was automatically generated.
// "See generate.py" for details.

#ifndef ENACT_STMT_H
#define ENACT_STMT_H

#include "../trivialStructs.h"

#include "Expr.h"

namespace enact {
    template<class R>
    class StmtVisitor;

    class Stmt {
    public:
        virtual ~Stmt() = default;

        // We need to overload for every possible visitor return type here, as we cannot
        // have a templated virtual member function.
        virtual std::string accept(StmtVisitor<std::string> *visitor) = 0;
        virtual void accept(StmtVisitor<void> *visitor) = 0;
    };

    class BreakStmt;
    class ContinueStmt;
    class ExpressionStmt;
    class FunctionStmt;
    class ReturnStmt;
    class StructStmt;
    class TraitStmt;
    class VariableStmt;

    template<class R>
    class StmtVisitor {
    public:
        R visitStmt(Stmt& stmt) {
            return stmt.accept(*this);
        };

        virtual R visitBreakStmt(BreakStmt &stmt) = 0;
        virtual R visitContinueStmt(ContinueStmt &stmt) = 0;
        virtual R visitExpressionStmt(ExpressionStmt &stmt) = 0;
        virtual R visitFunctionStmt(FunctionStmt &stmt) = 0;
        virtual R visitReturnStmt(ReturnStmt &stmt) = 0;
        virtual R visitStructStmt(StructStmt &stmt) = 0;
        virtual R visitTraitStmt(TraitStmt &stmt) = 0;
        virtual R visitVariableStmt(VariableStmt &stmt) = 0;
    };

    class BreakStmt : public Stmt {
    public:
        Token keyword;
        std::unique_ptr<Expr> value;

        BreakStmt(Token keyword) :
                keyword{std::move(keyword)},
                value{std::move(value)} {}

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

    class FunctionStmt : public Stmt {
    public:
        Token name;
        std::unique_ptr<const Typename> returnTypename;
        std::vector<Param> params;
        std::vector<std::unique_ptr<Stmt>> body;
        bool isMut;

        FunctionStmt(Token name, std::unique_ptr<const Typename> returnTypename, std::vector<Param> &&params,
                     std::vector<std::unique_ptr<Stmt>> body) :
                name{name},
                returnTypename{std::move(returnTypename)},
                params{std::move(params)},
                body{std::move(body)} {}

        ~FunctionStmt() override = default;

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitFunctionStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitFunctionStmt(*this);
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

        StructStmt(Token name, std::vector<Token> traits, std::vector<Field> &&fields,
                   std::vector<std::unique_ptr<FunctionStmt>> methods,
                   std::vector<std::unique_ptr<FunctionStmt>> assocFunctions) :
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

    class VariableStmt : public Stmt {
    public:
        Token name;
        std::unique_ptr<const Typename> typeName;
        std::unique_ptr<Expr> initializer;

        VariableStmt(Token name, std::unique_ptr<const Typename> typeName, std::unique_ptr<Expr> initializer) :
                name{name},
                typeName{std::move(typeName)},
                initializer{std::move(initializer)} {}

        ~VariableStmt() override = default;

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitVariableStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitVariableStmt(*this);
        }
    };
}

#endif // ENACT_STMT_H
