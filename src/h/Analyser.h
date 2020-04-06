#ifndef ENACT_ANALYSER_H
#define ENACT_ANALYSER_H

#include "../ast/Stmt.h"
#include "Type.h"

class Context;

// Walks the AST and assigns a Type to each node.

class Analyser : private StmtVisitor<void>, private ExprVisitor<void> {
    Context& m_context;

    std::unordered_map<std::string, Type> m_types{
            std::pair("int", INT_TYPE),
            std::pair("float", FLOAT_TYPE),
            std::pair("bool", BOOL_TYPE),
            std::pair("String", STRING_TYPE),
            std::pair("any", DYNAMIC_TYPE),
            std::pair("nothing", NOTHING_TYPE),
    };

    struct Variable {
        Type type = nullptr;
        bool isConst = false;
    };

    std::vector<std::unordered_map<std::string, Variable>> m_scopes{};

    // Keep track of whether we can use break/continue
    uint32_t m_loopCount = 0;

    // Keep track of the current function type to see if return statements are valid. Acts like a stack for nested
    // functions. If the stack is empty, then we are at the global scope.
    std::vector<FunctionType> m_currentFunctions{};

    // Keep track of functions that need to be analysed later
    std::vector<std::reference_wrapper<FunctionStmt>> m_globalFunctions{};

    bool m_hadError = false;

    void analyse(Stmt& stmt);
    void analyse(Expr& expr);

    void analyseFunctionBody(FunctionStmt& stmt);

    Type getFunctionType(const FunctionStmt &stmt);

    Variable& lookUpVariable(const Token& name);
    void declareVariable(const std::string& name, const Variable& variable);

    void beginScope();
    void endScope();

    class AnalysisError : public std::runtime_error {
    public:
        AnalysisError() : std::runtime_error{"Internal error, raising exception:\nUncaught AnalysisError! Private Analyser::analyse() was likely called by mistake where public Analyser::analyse() was supposed to be called instead."} {}
    };

    AnalysisError errorAt(const Token &token, const std::string &message);

    void visitBlockStmt(BlockStmt& stmt) override;
    void visitBreakStmt(BreakStmt& stmt) override;
    void visitContinueStmt(ContinueStmt& stmt) override;
    void visitEachStmt(EachStmt& stmt) override;
    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitFunctionStmt(FunctionStmt& stmt) override;
    void visitGivenStmt(GivenStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
    void visitStructStmt(StructStmt& stmt) override;
    void visitTraitStmt(TraitStmt& stmt) override;
    void visitWhileStmt(WhileStmt& stmt) override;
    void visitVariableStmt(VariableStmt& stmt) override;

    void visitAllotExpr(AllotExpr& expr) override;
    void visitAnyExpr(AnyExpr& expr) override;
    void visitArrayExpr(ArrayExpr& expr) override;
    void visitAssignExpr(AssignExpr& expr) override;
    void visitBinaryExpr(BinaryExpr& expr) override;
    void visitBooleanExpr(BooleanExpr& expr) override;
    void visitCallExpr(CallExpr& expr) override;
    void visitFloatExpr(FloatExpr& expr) override;
    void visitGetExpr(GetExpr& expr) override;
    void visitIntegerExpr(IntegerExpr& expr) override;
    void visitLogicalExpr(LogicalExpr& expr) override;
    void visitNilExpr(NilExpr& expr) override;
    void visitSetExpr(SetExpr& expr) override;
    void visitStringExpr(StringExpr& expr) override;
    void visitSubscriptExpr(SubscriptExpr& expr) override;
    void visitTernaryExpr(TernaryExpr& expr) override;
    void visitUnaryExpr(UnaryExpr& expr) override;
    void visitVariableExpr(VariableExpr& expr) override;

public:
    Analyser(Context& context);
    ~Analyser() = default;

    std::vector<std::unique_ptr<Stmt>> analyse(std::vector<std::unique_ptr<Stmt>> ast);

    Type lookUpType(const Typename& name);

    bool hadError();
};

#endif //ENACT_ANALYSER_H
