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
    class EnumStmt;
    class ExpressionStmt;
    class FunctionStmt;
    class ImplStmt;
    class ModuleStmt;
    class ReturnStmt;
    class StructStmt;
    class TraitStmt;
    class VariableStmt;

    template<class R>
    class StmtVisitor {
    public:
        R visitStmt(Stmt& stmt) {
            return stmt.accept(this);
        };

        virtual R visitBreakStmt(BreakStmt &stmt) = 0;
        virtual R visitContinueStmt(ContinueStmt &stmt) = 0;
        virtual R visitEnumStmt(EnumStmt& stmt) = 0;
        virtual R visitExpressionStmt(ExpressionStmt &stmt) = 0;
        virtual R visitFunctionStmt(FunctionStmt &stmt) = 0;
        virtual R visitImplStmt(ImplStmt& stmt) = 0;
        virtual R visitModuleStmt(ModuleStmt& stmt) = 0;
        virtual R visitReturnStmt(ReturnStmt &stmt) = 0;
        virtual R visitStructStmt(StructStmt &stmt) = 0;
        virtual R visitTraitStmt(TraitStmt &stmt) = 0;
        virtual R visitVariableStmt(VariableStmt &stmt) = 0;
    };

    class BreakStmt : public Stmt {
    public:
        Token keyword;
        std::unique_ptr<Expr> value;

        BreakStmt(Token keyword, std::unique_ptr<Expr> value) :
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

        explicit ContinueStmt(Token keyword) :
                keyword{std::move(keyword)} {}

        ~ContinueStmt() override = default;

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitContinueStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitContinueStmt(*this);
        }
    };

    class EnumStmt : public Stmt {
    public:
        struct Variant {
            Token name;
            std::unique_ptr<const Typename> typename_;
        };

        Token name;
        std::vector<Variant> variants;

        explicit EnumStmt(Token name, std::vector<Variant>&& variants) :
                name{std::move(name)},
                variants{std::move(variants)} {}

        ~EnumStmt() override = default;

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitEnumStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitEnumStmt(*this);
        }
    };

    class ExpressionStmt : public Stmt {
    public:
        std::unique_ptr<Expr> expr;

        explicit ExpressionStmt(std::unique_ptr<Expr> expr) :
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
        struct Param {
            Token name;
            std::unique_ptr<const Typename> typename_;
        };

        Token name;
        std::unique_ptr<const Typename> returnTypename;
        std::vector<Param> params;
        std::unique_ptr<BlockExpr> body;

        FunctionStmt(Token name, std::unique_ptr<const Typename> returnTypename, std::vector<Param> &&params,
                     std::unique_ptr<BlockExpr> body) :
                name{std::move(name)},
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

    class ImplStmt : public Stmt {
    public:
        std::unique_ptr<const Typename> typename_;
        std::unique_ptr<const Typename> traitTypename;
        std::vector<std::unique_ptr<FunctionStmt>> methods;

        ImplStmt(std::unique_ptr<const Typename> typename_,
                std::unique_ptr<const Typename> traitTypename,
                std::vector<std::unique_ptr<FunctionStmt>> methods) :
                typename_{std::move(typename_)},
                traitTypename{std::move(traitTypename)},
                methods{std::move(methods)} {}

        ~ImplStmt() override = default;

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitImplStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitImplStmt(*this);
        }
    };

    class ModuleStmt : public Stmt {
    public:
        // std::vector<std::unique_ptr<ImportStmt>> imports;
        std::vector<std::unique_ptr<Stmt>> decls;

        ModuleStmt(std::vector<std::unique_ptr<Stmt>> decls) :
                decls{std::move(decls)} {}

        std::string accept(StmtVisitor<std::string> *visitor) override {
            return visitor->visitModuleStmt(*this);
        }

        void accept(StmtVisitor<void> *visitor) override {
            return visitor->visitModuleStmt(*this);
        }
    };

    class ReturnStmt : public Stmt {
    public:
        Token keyword;
        std::unique_ptr<Expr> value;

        ReturnStmt(Token keyword, std::unique_ptr<Expr> value) :
                keyword{std::move(keyword)},
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
        struct Field {
            Token name;
            std::unique_ptr<const Typename> typename_;
        };

        Token name;
        std::vector<Field> fields;

        StructStmt(Token name, std::vector<Field>&& fields) :
                name{std::move(name)},
                fields{std::move(fields)} {}

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
                name{std::move(name)},
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
        Token keyword;
        Token name;
        std::unique_ptr<const Typename> typeName;
        std::unique_ptr<Expr> initializer;

        VariableStmt(
                Token keyword,
                Token name,
                std::unique_ptr<const Typename> typeName,
                std::unique_ptr<Expr> initializer) :
                keyword{std::move(keyword)},
                name{std::move(name)},
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
