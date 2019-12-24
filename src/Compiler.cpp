#include "h/Compiler.h"
#include "h/Object.h"
#include "h/Enact.h"

const Chunk& Compiler::compile(std::vector<Stmt> ast) {
    m_hadError = false;

    m_currentFunction = std::make_shared<FunctionObject>(
            std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{}),
            Chunk(),
            ""
    );

    beginScope();
    for (auto& stmt : ast) {
        compile(stmt);
    }
    endScope();

    emitByte(OpCode::RETURN);

    return currentChunk();
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
    beginScope();
    compile(stmt.initializer);

    size_t loopStartIndex = currentChunk().getCount();

    compile(stmt.condition);
    if (stmt.condition->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_BOOL);
    }

    size_t exitJumpIndex = emitJump(OpCode::JUMP_IF_FALSE);

    emitByte(OpCode::POP);

    beginScope();
    for (Stmt& statement : stmt.body) {
        compile(statement);
    }
    endScope();

    compile(stmt.increment);
    endScope();

    // Pop the increment
    emitByte(OpCode::POP);

    emitLoop(loopStartIndex, stmt.keyword);

    patchJump(exitJumpIndex, stmt.keyword);

    emitByte(OpCode::POP);
}

void Compiler::visitFunctionStmt(FunctionStmt &stmt) {
    throw errorAt(stmt.name, "Not implemented.");
}

void Compiler::visitGivenStmt(GivenStmt &stmt) {
    struct Jump {
        size_t index;
        Token where;
    };

    std::vector<Jump> exitJumps;

    for (GivenCase& case_ : stmt.cases) {
        compile(stmt.value);
        compile(case_.value);
        emitByte(OpCode::EQUAL);

        size_t jumpToNext = emitJump(OpCode::JUMP_IF_FALSE);

        emitByte(OpCode::POP);

        beginScope();
        for (Stmt& statement : case_.body) {
            compile(statement);
        }
        endScope();

        exitJumps.push_back(Jump{
                emitJump(OpCode::JUMP_IF_FALSE),
                case_.keyword
        });

        patchJump(jumpToNext, case_.keyword);
        emitByte(OpCode::POP);
    }

    for (Jump& jump : exitJumps) {
        patchJump(jump.index, jump.where);
    }
}

void Compiler::visitIfStmt(IfStmt &stmt) {
    compile(stmt.condition);
    if (stmt.condition->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_BOOL);
    }

    // Jump to the else branch if condition is false
    size_t thenJumpIndex = emitJump(OpCode::JUMP_IF_FALSE);

    // Pop the condition
    emitByte(OpCode::POP);

    beginScope();
    for (Stmt& statement : stmt.thenBlock) {
        compile(statement);
    }
    endScope();

    // If condition was true, we'll jump to the end of the else block
    size_t elseJumpIndex = emitJump(OpCode::JUMP);

    // If condition was false, we jumped here
    patchJump(thenJumpIndex, stmt.keyword);

    // Make sure we still pop the condition
    emitByte(OpCode::POP);

    beginScope();
    for (Stmt& statement : stmt.elseBlock) {
        compile(statement);
    }
    endScope();

    // If condition was true, we jumped here
    patchJump(elseJumpIndex, stmt.keyword);
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
    size_t loopStartIndex = currentChunk().getCount();

    compile(stmt.condition);
    if (stmt.condition->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_BOOL);
    }

    size_t exitJumpIndex = emitJump(OpCode::JUMP_IF_FALSE);

    emitByte(OpCode::POP);

    beginScope();
    for (Stmt& statement : stmt.body) {
        compile(statement);
    }
    endScope();

    emitLoop(loopStartIndex, stmt.keyword);

    patchJump(exitJumpIndex, stmt.keyword);

    emitByte(OpCode::POP);
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

    if (expr.oper.type != TokenType::EQUAL
            && expr.oper.type != TokenType::BANG_EQUAL
            && expr.right->getType()->isDynamic()) {
        emitByte(OpCode::CHECK_NUMERIC);
    }

    switch (expr.oper.type) {
        case TokenType::PLUS: emitByte(OpCode::ADD); break;
        case TokenType::MINUS: emitByte(OpCode::SUBTRACT); break;
        case TokenType::STAR: emitByte(OpCode::MULTIPLY); break;
        case TokenType::SLASH: emitByte(OpCode::DIVIDE); break;

        case TokenType::LESS: emitByte(OpCode::LESS); break;
        case TokenType::LESS_EQUAL:
            emitByte(OpCode::GREATER);
            emitByte(OpCode::NOT);
            break;

        case TokenType::GREATER: emitByte(OpCode::GREATER); break;
        case TokenType::GREATER_EQUAL:
            emitByte(OpCode::LESS);
            emitByte(OpCode::NOT);
            break;

        case TokenType::EQUAL_EQUAL: emitByte(OpCode::EQUAL); break;
        case TokenType::BANG_EQUAL:
            emitByte(OpCode::EQUAL);
            emitByte(OpCode::NOT);
            break;
    }
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
    // Always compile the left operand
    compile(expr.left);

    size_t jumpIndex = 0;

    if (expr.oper.type == TokenType::OR) {
        jumpIndex = emitJump(OpCode::JUMP_IF_TRUE);
    } else if (expr.oper.type == TokenType::AND) {
        jumpIndex = emitJump(OpCode::JUMP_IF_FALSE);
    }

    emitByte(OpCode::POP);

    compile(expr.right);

    patchJump(jumpIndex, expr.oper);
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
    compile(expr.operand);

    switch (expr.oper.type) {
        case TokenType::MINUS: {
            if (expr.operand->getType()->isDynamic()) {
                emitByte(OpCode::CHECK_NUMERIC);
            }
            emitByte(OpCode::NEGATE);

            break;
        }
        case TokenType::BANG: {
            if (expr.operand->getType()->isDynamic()) {
                emitByte(OpCode::CHECK_BOOL);
            }
            emitByte(OpCode::NOT);

            break;
        }
    }
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

void Compiler::beginScope() {
    ++m_scopeDepth;
}

void Compiler::endScope() {
    --m_scopeDepth;

    while (!m_variables.empty() && m_variables.back().depth > m_scopeDepth) {
        currentChunk().write(OpCode::POP, currentChunk().getCurrentLine());
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

void Compiler::emitByte(uint8_t byte) {
    currentChunk().write(byte, currentChunk().getCurrentLine());
}

void Compiler::emitByte(OpCode byte) {
    currentChunk().write(byte, currentChunk().getCurrentLine());
}

void Compiler::emitShort(uint16_t value) {
    currentChunk().writeShort(value, currentChunk().getCurrentLine());
}

void Compiler::emitLong(uint32_t value) {
    currentChunk().writeLong(value, currentChunk().getCurrentLine());
}

void Compiler::emitConstant(Value constant) {
    currentChunk().writeConstant(constant, currentChunk().getCurrentLine());
}

size_t Compiler::emitJump(OpCode jump) {
    emitByte(jump);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk().getCount() - 2;
}

void Compiler::patchJump(size_t index, Token where) {
    size_t jumpSize = currentChunk().getCount() - index - 2;

    if (jumpSize > UINT16_MAX) {
        throw errorAt(where, "Too much code in control flow block.");
    }

    currentChunk().rewrite(index, jumpSize & 0xff);
    currentChunk().rewrite(index + 1, (jumpSize >> 8) & 0xff);
}

void Compiler::emitLoop(size_t loopStartIndex, Token where) {
    emitByte(OpCode::LOOP);

    size_t jumpSize = currentChunk().getCount() - loopStartIndex + 2;
    if (jumpSize > UINT16_MAX) {
        throw errorAt(where, "Too much code in loop body.");
    }

    emitByte(jumpSize & 0xff);
    emitByte((jumpSize >> 8) & 0xff);
}

Chunk& Compiler::currentChunk() {
    return m_currentFunction->getChunk();
}
