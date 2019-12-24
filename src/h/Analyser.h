#ifndef ENACT_ANALYSER_H
#define ENACT_ANALYSER_H

#include "../ast/Stmt.h"
#include "Type.h"

#include <unordered_map>

// Walks the AST and assigns a Type to each node.

class Analyser : private StmtVisitor<void>, private ExprVisitor<void> {
    class AnalysisError : public std::runtime_error {
    public:
        AnalysisError() : std::runtime_error{"Internal error, raising exception:\nUncaught AnalysisError! Private Analyser::analyse() was likely called by mistake where public Analyser::analyse() was supposed to be called instead."} {}
    };

    struct Variable {
        Type type = nullptr;
        bool isConst = false;
    };

    std::unordered_map<std::string, Type> m_types{
            std::pair("int", INT_TYPE),
            std::pair("float", FLOAT_TYPE),
            std::pair("bool", BOOL_TYPE),
            std::pair("string", STRING_TYPE),
            std::pair("any", DYNAMIC_TYPE),
            std::pair("nothing", NOTHING_TYPE),
    };

    std::vector<std::unordered_map<std::string, Variable>> m_scopes{std::unordered_map<std::string, Variable>{}};
    bool m_hadError = false;

    // Keep track of whether we can use break/continue
    bool m_insideLoop = false;

    // Keep track of the current function type to see if return statements are valid
    std::optional<FunctionType> m_currentFunction = {};

    // Keep track of functions that need to be analysed later
    std::vector<FunctionStmt> m_globalFunctions;

    void analyse(Stmt stmt);
    void analyse(Expr expr);

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

    void visitAnyExpr(AnyExpr& expr) override;
    void visitArrayExpr(ArrayExpr& expr) override;
    void visitAssignExpr(AssignExpr& expr) override;
    void visitBinaryExpr(BinaryExpr& expr) override;
    void visitBooleanExpr(BooleanExpr& expr) override;
    void visitCallExpr(CallExpr& expr) override;
    void visitFieldExpr(FieldExpr& expr) override;
    void visitFloatExpr(FloatExpr& expr) override;
    void visitIntegerExpr(IntegerExpr& expr) override;
    void visitLogicalExpr(LogicalExpr& expr) override;
    void visitNilExpr(NilExpr& expr) override;
    void visitStringExpr(StringExpr& expr) override;
    void visitSubscriptExpr(SubscriptExpr& expr) override;
    void visitTernaryExpr(TernaryExpr& expr) override;
    void visitUnaryExpr(UnaryExpr& expr) override;
    void visitVariableExpr(VariableExpr& expr) override;

    void analyseFunctionBody(FunctionStmt& stmt);

    Type getFunctionType(const FunctionStmt &stmt);

    Type lookUpType(const Token& name);
    Type lookUpType(const std::string& name, const Token& where);

    Variable& lookUpVariable(const Token& name);
    void declareVariable(const std::string& name, const Variable& variable);

    void beginScope();
    void endScope();

public:
    void analyse(std::vector<Stmt> program);
    bool hadError();
};

#endif //ENACT_ANALYSER_H
