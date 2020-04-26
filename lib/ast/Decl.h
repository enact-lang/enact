#ifndef ENACT_DECL_H
#define ENACT_DECL_H

#include "../sema/VariableInfo.h"
#include "../trivialStructs.h"
#include "../type/Type.h"

#include "Expr.h"

namespace enact {
    template<class R>
    class DeclVisitor;

    class Decl {
    public:
        virtual ~Decl() = default;

        // We need to overload for every possible visitor return type here, as we cannot
        // have a templated virtual member function.
        virtual std::string accept(DeclVisitor<std::string> *visitor) = 0;
        virtual void accept(DeclVisitor<void> *visitor) = 0;
    };

    class ExpressionDecl;
    class FunctionDecl;
    class StructDecl;
    class TraitDecl;
    class VariableDecl;

    template<class R>
    class DeclVisitor {
    public:
        virtual R visit(Decl& decl) {
            return decl.accept(*this);
        }

        virtual R visitExpressionDecl(ExpressionDecl& decl) = 0;
        virtual R visitFunctionDecl(FunctionDecl& decl) = 0;
        virtual R visitStructDecl(StructDecl &decl) = 0;
        virtual R visitTraitDecl(TraitDecl &decl) = 0;
        virtual R visitVariableDecl(VariableDecl &decl) = 0;
    };

    class ExpressionDecl : public Decl {
    public:
        std::unique_ptr<Expr> expr;

        ExpressionDecl(std::unique_ptr<Expr> expr) :
                expr{std::move(expr)} {}

        ~ExpressionDecl() override = default;

        std::string accept(DeclVisitor<std::string> *visitor) override {
            return visitor->visitExpressionDecl(*this);
        }

        void accept(DeclVisitor<void> *visitor) override {
            return visitor->visitExpressionDecl(*this);
        }
    };

    enum class MutatesSelf {
        YES,
        NO,
        INVALID
    };

    class FunctionDecl : public Decl {
    public:
        Token name;
        std::unique_ptr<const Typename> returnTypename;
        std::vector<Param> params;
        std::vector<std::unique_ptr<Stmt>> body;
        Type type;
        MutatesSelf doesMutateSelf;

        FunctionDecl(Token name, std::unique_ptr<const Typename> returnTypename, std::vector<Param> &&params,
                     std::vector<std::unique_ptr<Stmt>> body, Type type) :
                name{name},
                returnTypename{std::move(returnTypename)},
                params{std::move(params)},
                body{std::move(body)},
                type{type},
                doesMutateSelf{MutatesSelf::INVALID} {}

        ~FunctionDecl() override = default;

        std::string accept(DeclVisitor<std::string> *visitor) override {
            return visitor->visitFunctionDecl(*this);
        }

        void accept(DeclVisitor<void> *visitor) override {
            return visitor->visitFunctionDecl(*this);
        }
    };

    class StructDecl : public Decl {
    public:
        Token name;
        std::vector<Token> traits;
        std::vector<Field> fields;
        std::vector<std::unique_ptr<FunctionDecl>> methods;
        std::vector<std::unique_ptr<FunctionDecl>> assocFunctions;
        std::shared_ptr<const ConstructorType> constructorType;

        StructDecl(Token name, std::vector<Token> traits, std::vector<Field> &&fields,
                   std::vector<std::unique_ptr<FunctionDecl>> methods,
                   std::vector<std::unique_ptr<FunctionDecl>> assocFunctions,
                   std::shared_ptr<const ConstructorType> constructorType) :
                name{name},
                traits{traits},
                fields{std::move(fields)},
                methods{std::move(methods)},
                assocFunctions{std::move(assocFunctions)},
                constructorType{constructorType} {}

        ~StructDecl() override = default;

        std::string accept(DeclVisitor<std::string> *visitor) override {
            return visitor->visitStructDecl(*this);
        }

        void accept(DeclVisitor<void> *visitor) override {
            return visitor->visitStructDecl(*this);
        }
    };

    class TraitDecl : public Decl {
    public:
        Token name;
        std::vector<std::unique_ptr<FunctionDecl>> methods;

        TraitDecl(Token name, std::vector<std::unique_ptr<FunctionDecl>> methods) :
                name{name},
                methods{std::move(methods)} {}

        ~TraitDecl() override = default;

        std::string accept(DeclVisitor<std::string> *visitor) override {
            return visitor->visitTraitDecl(*this);
        }

        void accept(DeclVisitor<void> *visitor) override {
            return visitor->visitTraitDecl(*this);
        }
    };

    class VariableDecl : public Decl {
    public:
        Token name;
        std::unique_ptr<const Typename> typeName;
        std::unique_ptr<Expr> initializer;
        Mutability mutability;

        VariableDecl(Token name, std::unique_ptr<const Typename> typeName, std::unique_ptr<Expr> initializer,
                     Mutability mutability) :
                name{name},
                typeName{std::move(typeName)},
                initializer{std::move(initializer)},
                mutability{mutability} {}

        ~VariableDecl() override = default;

        std::string accept(DeclVisitor<std::string> *visitor) override {
            return visitor->visitVariableDecl(*this);
        }

        void accept(DeclVisitor<void> *visitor) override {
            return visitor->visitVariableDecl(*this);
        }
    };
}

#endif //ENACT_DECL_H
