#include "h/Compiler.h"
#include "h/Enact.h"

const Chunk& Compiler::compile(std::vector<Stmt> ast) {
    m_hadError = false;

    for (auto& stmt : ast) {
        compile(stmt);
    }

    return m_chunk;
}

void Compiler::compile(Stmt stmt) {
    try {
        stmt->accept(this);
    } catch (CompileError& error) {
        m_hadError = true;
    }
}

void Compiler::compile(Expr expr) {
    expr->accept(this);
}

void Compiler::visitBlockStmt(BlockStmt &stmt) {
    throw CompileError{};
}

void Compiler::visitBreakStmt(BreakStmt &stmt) {
    throw errorAt(stmt.keyword, "Not implemented.");
}

void Compiler::visitContinueStmt(ContinueStmt &stmt) {
    throw errorAt(stmt.keyword, "Not implemented.");
}

void Compiler::visitEachStmt(EachStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitExpressionStmt(ExpressionStmt &stmt) {
    compile(stmt.expr);
    m_chunk.write(OpCode::POP, m_chunk.getCurrentLine());
}

void Compiler::visitForStmt(ForStmt &stmt) {
    throw CompileError{};
}

void Compiler::visitFunctionStmt(FunctionStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitGivenStmt(GivenStmt &stmt) {
    throw CompileError{};
}

void Compiler::visitIfStmt(IfStmt &stmt) {
    throw errorAt(stmt.keyword, "Not implemented.");
}

void Compiler::visitReturnStmt(ReturnStmt &stmt) {
    throw errorAt(stmt.keyword, "Not implemented.");
}

void Compiler::visitStructStmt(StructStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitTraitStmt(TraitStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitWhileStmt(WhileStmt &stmt) {
    throw CompileError{};
}

void Compiler::visitVariableStmt(VariableStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitAnyExpr(AnyExpr &expr) {
    throw CompileError{};
}

void Compiler::visitArrayExpr(ArrayExpr &expr) {
    throw errorAt(expr.square, "Not implemented.");
}

void Compiler::visitAssignExpr(AssignExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitBinaryExpr(BinaryExpr &expr) {
    compile(expr.left);

    if (expr.left->getType()->isDynamic()) {
        m_chunk.write(OpCode::CHECK_NUMERIC, expr.oper.line);
    }

    compile(expr.right);

    if (expr.right->getType()->isDynamic()) {
        m_chunk.write(OpCode::CHECK_NUMERIC, expr.oper.line);
    }

    OpCode op;

    switch (expr.oper.type) {
        case TokenType::PLUS: op = OpCode::ADD; break;
        case TokenType::MINUS: op = OpCode::SUBTRACT; break;
        case TokenType::STAR: op = OpCode::MULTIPLY; break;
        case TokenType::SLASH: op = OpCode::DIVIDE; break;
    }

    m_chunk.write(op, expr.oper.line);
}

void Compiler::visitBooleanExpr(BooleanExpr &expr) {
    m_chunk.write(expr.value ? OpCode::TRUE : OpCode::FALSE, m_chunk.getCurrentLine());
}

void Compiler::visitCallExpr(CallExpr &expr) {
    throw errorAt(expr.paren, "Not implemented.");
}

void Compiler::visitFieldExpr(FieldExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitFloatExpr(FloatExpr &expr) {
    m_chunk.writeConstant(Value{expr.value}, m_chunk.getCurrentLine());
}

void Compiler::visitIntegerExpr(IntegerExpr &expr) {
    m_chunk.writeConstant(Value{expr.value}, m_chunk.getCurrentLine());
}

void Compiler::visitLogicalExpr(LogicalExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitNilExpr(NilExpr &expr) {
    m_chunk.write(OpCode::NIL, m_chunk.getCurrentLine());
}

void Compiler::visitStringExpr(StringExpr &expr) {
    throw CompileError{};
}

void Compiler::visitSubscriptExpr(SubscriptExpr &expr) {
    throw errorAt(expr.square, "Not implemented.");
}

void Compiler::visitTernaryExpr(TernaryExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitUnaryExpr(UnaryExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitVariableExpr(VariableExpr &expr) {
    throw errorAt(expr.name, "Not implemented.");
}

Compiler::CompileError Compiler::errorAt(const Token &token, const std::string &message) {
    Enact::reportErrorAt(token, message);
    m_hadError = true;
    return CompileError{};
}

bool Compiler::hadError() {
    return m_hadError;
}
