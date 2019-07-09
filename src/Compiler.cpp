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

    m_chunk.write(OpCode::RETURN, m_chunk.getCurrentLine());

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
            m_chunk.write(OpCode::SET_VARIABLE, m_chunk.getCurrentLine());
            m_chunk.write(static_cast<uint8_t>(index), m_chunk.getCurrentLine());
        } else {
            m_chunk.write(OpCode::SET_VARIABLE_LONG, m_chunk.getCurrentLine());
            m_chunk.writeLong(index, m_chunk.getCurrentLine());
        }
    } else {
        throw errorAt(expr.oper, "Not implemented.");
    }
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
    Object* string = new StringObject{expr.value};
    m_chunk.writeConstant(Value{string}, m_chunk.getCurrentLine());
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
        m_chunk.write(OpCode::GET_VARIABLE, m_chunk.getCurrentLine());
        m_chunk.write(static_cast<uint8_t>(index), m_chunk.getCurrentLine());
    } else {
        m_chunk.write(OpCode::GET_VARIABLE_LONG, m_chunk.getCurrentLine());
        m_chunk.writeLong(index, m_chunk.getCurrentLine());
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
