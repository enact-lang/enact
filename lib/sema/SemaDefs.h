#ifndef ENACT_SEMADEFS_H
#define ENACT_SEMADEFS_H

namespace enact {
    // Forward declared from "Sema.h"
    class Sema;

    // The second and final pass over the AST conducted by Sema.
    // Using the type and variable declarations which SemaDecls so thoughtfully assembled
    // for us, we walk the AST again, hopefully giving a complete type to every declaration.

    // While doing this, we assign types to each node of the AST and perform typechecking,
    // including both declaration and definition for non-global scopes (we can do this all
    // here because local declarations must be in order).

    // If we come across a declaration that has not yet been defined, we first attempt to
    // define it, and if that is unsuccessful, we report an error.
    class SemaDefs {
    public:
        explicit SemaDefs(Sema& sema);

        std::vector<std::unique_ptr<Decl>> define(std::vector<std::unique_ptr<Decl> ast);

    private:
        Sema& m_sema;

        // All local variables and their corresponding types and semantic info. Each element
        // in the list, starting from the front, is one local scope, with the outermost global
        // scope stored in Sema.
        std::list<std::unordered_map<std::string, VariableInfo>> m_localVariables;

        // All local type declarations and their corresponding values. Again, each element in
        // the list, starting from the front, is one local scope, with the outermost global
        // scope stored in Sema.
        std::list<std::unordered_map<std::string, Type>> m_localTypes;

        // Keep track of the type of the current function to see if return statements are valid.
        std::vector<std::shared_ptr<const FunctionType>> m_currentFunctions;

        // Push/pop a new local scope to both `m_localTypes` and `m_localVariables`. We start
        // in the global scope stored by Sema.
        void beginScope();
        void endScope();

        // Declare a local symbol in the scope specified by `depth`. Starts from the current scope,
        // i.e. the default, 0, is the innermost scope.
        void declareLocalVariable(const std::string& name, const VariableInfo& info, size_t depth = 0);
        void declareLocalType(const std::string& name, Type value = nullptr, size_t depth = 0);

        // Define a local symbol in the scope specified by `depth`.
        void defineLocalVariable(const std::string& name, Type type, size_t depth = 0);
        void defineLocalType(const std::string& name, Type value, size_t depth = 0);

        // Look up a symbol, starting in the innermost scope and searching all the way out, including
        // in the global scope stored by Sema. Returns `nullopt` if the symbol has not been declared,
        // otherwise returns the symbol in question.
        std::optional<VariableInfo> variableDeclared(const std::string& name);
        std::optional<Type> typeDeclared(const std::string& name);

        void visitBlockStmt(BlockStmt &stmt) override;
        void visitBreakStmt(BreakStmt &stmt) override;
        void visitContinueStmt(ContinueStmt &stmt) override;
        void visitEachStmt(EachStmt &stmt) override;
        void visitDeclarationStmt(DeclarationStmt &stmt) override;
        void visitForStmt(ForStmt &stmt) override;
        void visitGivenStmt(GivenStmt &stmt) override;
        void visitIfStmt(IfStmt &stmt) override;
        void visitReturnStmt(ReturnStmt &stmt) override;
        void visitWhileStmt(WhileStmt &stmt) override;

        void visitExpressionDecl(ExpressionDecl& decl) override;
        void visitFunctionDecl(FunctionDecl& decl) override;
        void visitStructDecl(StructDecl& decl) override;
        void visitTraitDecl(TraitDecl& decl) override;
        void visitVariableDecl(VariableDecl& decl) override;

        void visitAllotExpr(AllotExpr &expr) override;
        void visitAnyExpr(AnyExpr &expr) override;
        void visitArrayExpr(ArrayExpr &expr) override;
        void visitAssignExpr(AssignExpr &expr) override;
        void visitBinaryExpr(BinaryExpr &expr) override;
        void visitBooleanExpr(BooleanExpr &expr) override;
        void visitCallExpr(CallExpr &expr) override;
        void visitFloatExpr(FloatExpr &expr) override;
        void visitGetExpr(GetExpr &expr) override;
        void visitIntegerExpr(IntegerExpr &expr) override;
        void visitLogicalExpr(LogicalExpr &expr) override;
        void visitNilExpr(NilExpr &expr) override;
        void visitSetExpr(SetExpr &expr) override;
        void visitStringExpr(StringExpr &expr) override;
        void visitSubscriptExpr(SubscriptExpr &expr) override;
        void visitTernaryExpr(TernaryExpr &expr) override;
        void visitUnaryExpr(UnaryExpr &expr) override;
        void visitVariableExpr(VariableExpr &expr) override;
    };
}

#endif //ENACT_SEMADEFS_H
