#include "h/Compiler.h"
#include "h/Object.h"
#include "h/Enact.h"
#include "h/Natives.h"

Compiler::Compiler(Compiler* enclosing) : m_enclosing{enclosing} {}

void Compiler::init(FunctionKind functionKind, Type functionType, const std::string& name) {
    m_hadError = false;

    m_currentFunction = new FunctionObject{
            functionType,
            Chunk(),
            name
    };

    m_functionType = functionKind;

    m_scopeDepth = 0;

    beginScope();

    addLocal(Token{TokenType::IDENTIFIER, "", 0, 0});

    if (functionKind == FunctionKind::SCRIPT) {
        defineNative("print", std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{DYNAMIC_TYPE}),
                     &Natives::print);
        defineNative("put", std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{DYNAMIC_TYPE}),
                     &Natives::put);
        defineNative("dis", std::make_shared<FunctionType>(STRING_TYPE, std::vector<Type>{DYNAMIC_TYPE}),
                     &Natives::dis);
    }
}

FunctionObject* Compiler::end() {
    emitByte(OpCode::NIL);
    emitByte(OpCode::RETURN);
    return m_currentFunction;
}

void Compiler::compile(std::vector<Stmt> ast) {
    for (auto& stmt : ast) {
        compile(stmt);
    }
}

void Compiler::compile(Stmt stmt) {
    try {
        stmt->accept(this);
    } catch (CompileError& error) {
        Enact::reportErrorAt(error.getToken(), error.getMessage());
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
    addLocal(stmt.name);
    m_locals.back().initialized = true;

    Compiler compiler{this};
    compiler.init(FunctionKind::FUNCTION, stmt.type, stmt.name.lexeme);

    for (const NamedTypename& param : stmt.params) {
        compiler.addLocal(param.name);
        compiler.m_locals.back().initialized = true;
    }

    compiler.compile(stmt.body);
    FunctionObject* function = compiler.end();

    uint32_t constantIndex = currentChunk().addConstant(Value{function});
    if (constantIndex < UINT8_MAX) {
        emitByte(OpCode::CLOSURE);
        emitByte(constantIndex);
    } else {
        emitByte(OpCode::CLOSURE_LONG);
        emitLong(constantIndex);
    }

    for (int i = 0; i < function->getUpvalueCount(); i++) {
        emitByte(compiler.m_upvalues[i].isLocal ? 1 : 0);

        if (i < UINT8_MAX) {
            emitByte(static_cast<uint8_t>(compiler.m_upvalues[i].index));
        } else {
            emitLong(compiler.m_upvalues[i].index);
        }
    }
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
    compile(stmt.value);
    emitByte(OpCode::RETURN);
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
    addLocal(stmt.name);
    compile(stmt.initializer);
    m_locals.back().initialized = true;
}

void Compiler::visitAnyExpr(AnyExpr &expr) {
    // TODO: Throw CompileError "Not implemented."
}

void Compiler::visitArrayExpr(ArrayExpr &expr) {
    throw errorAt(expr.square, "Not implemented.");
}

void Compiler::visitAssignExpr(AssignExpr &expr) {
    compile(expr.right);

    if (typeid(*expr.left) == typeid(VariableExpr)) {
        auto variableExpr = std::static_pointer_cast<VariableExpr>(expr.left);

        uint32_t index = resolveLocal(variableExpr->name);
        if (index <= UINT8_MAX) {
            emitByte(OpCode::SET_LOCAL);
            emitByte(static_cast<uint8_t>(index));
        } else {
            emitByte(OpCode::SET_LOCAL_LONG);
            emitLong(index);
        }
    } else {
        throw errorAt(expr.oper, "Not implemented.");
    }
}

void Compiler::visitBinaryExpr(BinaryExpr &expr) {
    compile(expr.left);

    if (expr.oper.type != TokenType::EQUAL
            && expr.oper.type != TokenType::BANG_EQUAL
            && expr.left->getType()->isDynamic()) {
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
    compile(expr.callee);

    bool needRuntimeCheck = expr.callee->getType()->isDynamic();

    for (int i = 0; i < expr.arguments.size(); ++i) {
        compile(expr.arguments[i]);
        if (expr.arguments[i]->getType()->isDynamic()) {
            needRuntimeCheck = true;
        }
    }

    if (needRuntimeCheck) {
        emitByte(OpCode::CHECK_CALLABLE);
        emitByte(expr.arguments.size());
    }

    emitByte(OpCode::CALL);
    emitByte(static_cast<uint8_t>(expr.arguments.size()));
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
    try {
        uint32_t index = resolveLocal(expr.name);
        if (index <= UINT8_MAX) {
            emitByte(OpCode::GET_LOCAL);
            emitByte(static_cast<uint8_t>(index));
        } else {
            emitByte(OpCode::GET_LOCAL_LONG);
            emitLong(index);
        }
        return;
    } catch (CompileError& error) {}

    uint32_t index = resolveUpvalue(expr.name);
    if (index <= UINT8_MAX) {
        emitByte(OpCode::GET_UPVALUE);
        emitByte(static_cast<uint8_t>(index));
    } else {
        emitByte(OpCode::GET_UPVALUE_LONG);
        emitLong(index);
    }

}

Compiler::CompileError Compiler::errorAt(const Token &token, const std::string &message) {
    return CompileError{token, message};
}

bool Compiler::hadError() {
    return m_hadError;
}

void Compiler::beginScope() {
    ++m_scopeDepth;
}

void Compiler::endScope() {
    --m_scopeDepth;

    while (!m_locals.empty() && m_locals.back().depth > m_scopeDepth) {
        if (m_locals.back().isCaptured) {
            emitByte(OpCode::CLOSE_UPVALUE);
        } else {
            emitByte(OpCode::POP);
        }
        m_locals.pop_back();
    }
}

void Compiler::addLocal(const Token& name) {
    m_locals.push_back(Local{
            name,
            m_scopeDepth,
            false,
            false
    });
}

uint32_t Compiler::resolveLocal(const Token& name) {
    for (int i = m_locals.size() - 1; i >= 0; --i) {
        const Local& local = m_locals[i];

        if (local.name.lexeme == name.lexeme) {
            if (!local.initialized) {
                throw errorAt(local.name, "Cannot reference a variable in its own initializer.");
            }
            return i;
        }
    }

    throw errorAt(name, "Could not resolve variable with name " + name.lexeme + ".");
}

void Compiler::addUpvalue(uint32_t index, bool isLocal) {
    m_upvalues.push_back(Upvalue{index, isLocal});
    m_currentFunction->getUpvalueCount()++;
}

uint32_t Compiler::resolveUpvalue(const Token &name) {
    if (m_enclosing == nullptr) {
        throw errorAt(name, "Could not resolve variable with name " + name.lexeme + ".");
    }

    try {
        uint32_t local = m_enclosing->resolveLocal(name);

        for (int i = 0; i < m_upvalues.size(); ++i) {
            Upvalue& upvalue = m_upvalues[i];
            if (upvalue.index == local && upvalue.isLocal) {
                return i;
            }
        }

        m_enclosing->m_locals[local].isCaptured = true;
        addUpvalue(local, true);
        return m_upvalues.size() - 1;
    } catch (CompileError& error) {}

    try {
        uint32_t upvalue = m_enclosing->resolveUpvalue(name);
        addUpvalue(upvalue, false);
        return m_upvalues.size() - 1;
    } catch (CompileError& error) {}

    throw errorAt(name, "Could not resolve variable with name " + name.lexeme + ".");
}

void Compiler::defineNative(std::string name, Type functionType, NativeFn function) {
    NativeObject* native = new NativeObject{functionType, function};
    emitConstant(Value{native});

    addLocal(Token{TokenType::IDENTIFIER, name, 0, 0});
    m_locals.back().initialized = true;
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
