#include "h/Compiler.h"
#include "h/Enact.h"

void Compiler::beginScope() {
    ++m_scopeDepth;
}

void Compiler::endScope() {
    --m_scopeDepth;

    while (!m_variables.empty() && m_variables.back().depth > m_scopeDepth) {
        m_chunk.write(OpCode::POP, m_chunk.getCurrentLine());
        m_variables.pop_back();
    }
}

void Compiler::addVariable(const Token& name) {
    m_variables.push_back(Variable{
        name,
        m_scopeDepth,
        false
    });
}

uint32_t Compiler::resolveVariable(const Token& name) {
    for (int i = m_variables.size() - 1; i >= 0; --i) {
        const Variable& variable = m_variables[i];

        if (variable.name.lexeme == name.lexeme) {
            if (!variable.initialized) {
                throw errorAt(variable.name, "Cannot reference a variable in its own initializer.");
            }
            return i;
        }
    }

    throw errorAt(name, "Could not resolve variable with name " + name.lexeme + ".");
}

const Chunk& Compiler::compile(std::vector<Stmt> ast) {
    m_hadError = false;

    beginScope();
    for (auto& stmt : ast) {
        compile(stmt);
    }
    endScope();

    emitByte(OpCode::RETURN);

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
    beginScope();
    for (const Stmt& statement : stmt.statements) {
        compile(statement);
    }
    endScope();
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
    emitByte(OpCode::POP);
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
    addVariable(stmt.name);
    compile(stmt.initializer);
    m_variables.back().initialized = true;
}

void Compiler::visitAnyExpr(AnyExpr &expr) {
    throw CompileError{};
}

void Compiler::visitArrayExpr(ArrayExpr &expr) {
    throw errorAt(expr.square, "Not implemented.");
}

void Compiler::visitAssignExpr(AssignExpr &expr) {
    compile(expr.right);

    if (typeid(*expr.left) == typeid(VariableExpr)) {
        auto variableExpr = std::static_pointer_cast<VariableExpr>(expr.left);

        uint32_t index = resolveVariable(variableExpr->name);
        if (index <= UINT8_MAX) {
            emitByte(OpCode::SET_VARIABLE);
            emitByte(static_cast<uint8_t>(index));
        } else {
            emitByte(OpCode::SET_VARIABLE_LONG);
            emitLong(index);
        }
    } else {
        throw errorAt(expr.oper, "Not implemented.");
    }
}

void Compiler::visitBinaryExpr(BinaryExpr &expr) {
    compile(expr.left);

    if (expr.left->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_NUMERIC);
    }

    compile(expr.right);

    if (expr.right->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_NUMERIC);
    }

    OpCode op;

    switch (expr.oper.type) {
        case TokenType::PLUS: op = OpCode::ADD; break;
        case TokenType::MINUS: op = OpCode::SUBTRACT; break;
        case TokenType::STAR: op = OpCode::MULTIPLY; break;
        case TokenType::SLASH: op = OpCode::DIVIDE; break;
    }

    emitByte(op);
}

void Compiler::visitBooleanExpr(BooleanExpr &expr) {
    emitByte(expr.value ? OpCode::TRUE : OpCode::FALSE);
}

void Compiler::visitCallExpr(CallExpr &expr) {
    throw errorAt(expr.paren, "Not implemented.");
}

void Compiler::visitFieldExpr(FieldExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitFloatExpr(FloatExpr &expr) {
    emitConstant(Value{expr.value});
}

void Compiler::visitIntegerExpr(IntegerExpr &expr) {
    emitConstant(Value{expr.value});
}

void Compiler::visitLogicalExpr(LogicalExpr &expr) {
    throw errorAt(expr.oper, "Not implemented.");
}

void Compiler::visitNilExpr(NilExpr &expr) {
    emitByte(OpCode::NIL);
}

void Compiler::visitStringExpr(StringExpr &expr) {
    Object* string = new StringObject{expr.value};
    emitConstant(Value{string});
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
    uint32_t index = resolveVariable(expr.name);
    if (index <= UINT8_MAX) {
        emitByte(OpCode::GET_VARIABLE);
        emitByte(static_cast<uint8_t>(index));
    } else {
        emitByte(OpCode::GET_VARIABLE_LONG);
        emitLong(index);
    }
}

Compiler::CompileError Compiler::errorAt(const Token &token, const std::string &message) {
    Enact::reportErrorAt(token, message);
    m_hadError = true;
    return CompileError{};
}

bool Compiler::hadError() {
    return m_hadError;
}

void Compiler::emitByte(uint8_t byte) {
    m_chunk.write(byte, m_chunk.getCurrentLine());
}

void Compiler::emitByte(OpCode byte) {
    m_chunk.write(byte, m_chunk.getCurrentLine());
}

void Compiler::emitLong(uint32_t value) {
    m_chunk.writeLong(value, m_chunk.getCurrentLine());
}

void Compiler::emitConstant(Value constant) {
    m_chunk.writeConstant(constant, m_chunk.getCurrentLine());
}
