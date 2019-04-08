#ifndef ENACT_COMPILER_H
#define ENACT_COMPILER_H

#include "../ast/Stmt.h"
#include "Chunk.h"

class Compiler : private StmtVisitor<void>, private ExprVisitor<void> {
    Chunk m_chunk;

    void compile(Stmt stmt);
    void compile(Expr expr);

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
    void visitReferenceExpr(ReferenceExpr& expr) override;
    void visitStringExpr(StringExpr& expr) override;
    void visitSubscriptExpr(SubscriptExpr& expr) override;
    void visitTernaryExpr(TernaryExpr& expr) override;
    void visitUnaryExpr(UnaryExpr& expr) override;
    void visitVariableExpr(VariableExpr& expr) override;


    class CompileError : public std::runtime_error {
    public:
        CompileError() : std::runtime_error{"Internal error, raising exception:\nUncaught CompileError!"} {}
    };

    bool m_hadError = false;

    CompileError errorAt(const Token &token, const std::string &message);
    
public:
    const Chunk& compile(std::vector<Stmt> ast);

    bool hadError();
};

#endif //ENACT_COMPILER_H
