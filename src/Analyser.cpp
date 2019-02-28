#include <vector>
#include <set>
#include "h/Analyser.h"
#include "h/Enact.h"

void Analyser::analyse(std::vector<Stmt> program) {
    m_hadError = false;

    for (auto &stmt : program) {
        // We catch errors at the top level.
        try {
            analyse(stmt);
        } catch (const AnalysisError& error) {
            continue;
        }
    }
}

void Analyser::analyse(Stmt stmt) {
     stmt->accept(this);
}

void Analyser::analyse(Expr expr) {
    expr->accept(this);
}

bool Analyser::hadError() {
    return m_hadError;
}

Analyser::AnalysisError Analyser::errorAt(const Token &token, const std::string &message) {
    Enact::reportErrorAt(token, message);
    m_hadError = true;
    return AnalysisError{};
}

void Analyser::visitBlockStmt(BlockStmt& stmt) {
    beginScope();
    for (Stmt& statement : stmt.statements) {
        analyse(statement);
    }
    endScope();
}

void Analyser::visitBreakStmt(BreakStmt& stmt) {
    if (!m_insideLoop) {
        throw errorAt(stmt.keyword, "Break is only allowed inside loops.");
    }
}

void Analyser::visitContinueStmt(ContinueStmt& stmt) {
    if (!m_insideLoop) {
        throw errorAt(stmt.keyword, "Continue is only allowed inside loops.");
    }
}

void Analyser::visitEachStmt(EachStmt& stmt) {
    throw errorAt(stmt.name, "Unsupported feature.");
}

void Analyser::visitExpressionStmt(ExpressionStmt& stmt) {
    analyse(stmt.expr);
}

void Analyser::visitForStmt(ForStmt& stmt) {
    analyse(stmt.initializer);
    analyse(stmt.condition);
    analyse(stmt.increment);

    m_insideLoop = true;
    beginScope();
    for (Stmt& statement : stmt.body) {
        analyse(statement);
    }
    endScope();
    m_insideLoop = false;
}

void Analyser::visitFunctionStmt(FunctionStmt& stmt) {
    Type type = getFunctionType(stmt);
    auto functionType = type->as<FunctionType>();

    declareVariable(stmt.name.lexeme, Variable{type, true});

    beginScope();
    m_currentFunction = type->as<FunctionType>();

    for (int i = 0; i < stmt.params.size(); ++i) {
        declareVariable(stmt.params[i].name.lexeme, Variable{functionType->getArgumentTypes()[i]});
    }

    for (Stmt& statement : stmt.body) {
        analyse(statement);
    }

    m_currentFunction = nullptr;
    endScope();

}

void Analyser::visitGivenStmt(GivenStmt& stmt) {
    analyse(stmt.value);
    Type valueType = stmt.value->getType();

    for (const GivenCase& case_ : stmt.cases) {
        analyse(case_.value);
        if (!case_.value->getType()->looselyEquals(*valueType)) {
            throw errorAt(case_.keyword, "Given value of type '" + valueType->toString()
                    + "' cannot be compared with case of type '" + case_.value->getType()->toString() + "'.");
        }

        beginScope();

        for (const Stmt& bodyStmt : case_.body) {
            analyse(bodyStmt);
        }

        endScope();
    }
}

void Analyser::visitIfStmt(IfStmt& stmt) {
    analyse(stmt.condition);

    beginScope();
    for (Stmt& statement : stmt.thenBlock) {
        analyse(statement);
    }
    endScope();

    beginScope();
    for (Stmt& statement : stmt.elseBlock) {
        analyse(statement);
    }
    endScope();
}

void Analyser::visitReturnStmt(ReturnStmt& stmt) {
    if (m_currentFunction == nullptr) {
        throw errorAt(stmt.keyword, "Return is only allowed inside functions.");
    }

    analyse(stmt.value);

    Type returnType = m_currentFunction->getReturnType();
    if (!returnType->looselyEquals(*stmt.value->getType())) {
        throw errorAt(stmt.keyword, "Cannot return from function with return type '" +
                returnType->toString() + "' with value of type '" + stmt.value->getType()->toString() + "'.");
    }
}

void Analyser::visitStructStmt(StructStmt& stmt) {
    if (m_types.count(stmt.name.lexeme) > 0) {
        throw errorAt(stmt.name, "Cannot redeclare type '" + stmt.name.lexeme + "'.");
    }

    std::vector<Type> traits;
    for (const Token &traitName : stmt.traits) {
        // Check that the trait has been declared as a type.
        if (m_types.count(traitName.lexeme) > 0) {
            // Check that the trait actually is a trait, and not an 'int' or something.
            if (m_types[traitName.lexeme]->isTrait()) {
                traits.push_back(m_types[traitName.lexeme]);
            } else {
                throw errorAt(traitName, "Type '" + traitName.lexeme + "' is not a trait.");
            }
        } else {
            throw errorAt(traitName, "Undeclared trait '" + traitName.lexeme + "'.");
        }
    }

    // In the AST, fields are represented as a name paired with a type (NamedTypename).
    // We must now find the types that the typenames are referring to, and create
    // NamedTypes containing them.
    std::unordered_map<std::string, Type> fields;
    std::vector<Type> fieldTypes;
    for (const NamedTypename& field : stmt.fields) {
        // Check if the field has the same name as another field
        if (fields.count(field.name.lexeme) > 0) {
            throw errorAt(field.name, "Struct fieldield '" + field.name.lexeme +
                    "' cannot have the same name as another field.");
        }

        if (m_types.count(field.typeName) > 0) {
            fields.insert(std::pair(field.name.lexeme, m_types[field.typeName]));
            fieldTypes.push_back(m_types[field.typeName]);
        } else {
            throw errorAt(field.name, "Undeclared field type '" + field.typeName + "'.");
        }
    }

    // Methods are must be kept seperate from fields, so we can declare the constructor
    // properly. We take the methods as they are represented in the AST (a pointer to
    // a FunctionStmt) and convert them to NamedTypes.
    std::unordered_map<std::string, Type> methods;
    for (const std::shared_ptr<FunctionStmt>& method : stmt.methods) {
        // Check if the method has the same name as a field
        if (methods.count(method->name.lexeme) > 0 || fields.count(method->name.lexeme) > 0) {
                throw errorAt(method->name, "Struct method '" + method->name.lexeme +
                        "' cannot have the same name as another field or method.");
        }

        methods.insert(std::pair(method->name.lexeme, getFunctionType(*method)));
    }

    // Assoc functions must be kept separate from the fields, as they are called on
    // the type rather than an instance of the type.
    std::unordered_map<std::string, Type> assocFunctions;

    Type thisType = std::make_shared<StructType>(stmt.name.lexeme, traits, fields, methods, assocFunctions);
    m_types.insert(std::make_pair(stmt.name.lexeme, thisType));

    for (const std::shared_ptr<FunctionStmt>& function : stmt.assocFunctions) {
        assocFunctions.insert(std::pair(function->name.lexeme, getFunctionType(*function)));
    }

    thisType = std::make_shared<StructType>(stmt.name.lexeme, traits, fields, methods, assocFunctions);

    m_types[stmt.name.lexeme] = thisType;

    // Now, create a constructor for the struct.
    Variable constructor{std::make_shared<ConstructorType>(*thisType->as<StructType>()), true};
    declareVariable(stmt.name.lexeme, constructor);

    // We now need to analyse all of the code inside the methods.
    // First, we'll begin the struct scope:
    beginScope();

    // Next, we'll declare all fields and methods:
    for (const auto& field : fields) {
        declareVariable(field.first, Variable{field.second, false});
    }

    for (const auto& method : methods) {
        declareVariable(method.first, Variable{method.second, true});
    }

    // And "this":
    declareVariable("this", Variable{thisType, false});

    // Now we can analyse the methods:
    for (const std::shared_ptr<FunctionStmt>& method : stmt.methods) {
        analyse(method);
    }

    // End the struct scope.
    endScope();

    // Look at the assoc functions (remember, they are outside of the struct scope):
    for (const std::shared_ptr<FunctionStmt>& function : stmt.assocFunctions) {
        analyse(function);
    }
}

void Analyser::visitTraitStmt(TraitStmt& stmt) {
    if (m_types.count(stmt.name.lexeme) > 0) {
        throw errorAt(stmt.name, "Cannot redeclare type '" + stmt.name.lexeme + "'.");
    }

    std::unordered_map<std::string, Type> methods;
    for (const std::shared_ptr<FunctionStmt>& method : stmt.methods) {
        if (methods.count(method->name.lexeme) > 0) {
            throw errorAt(method->name, "Trait method '" + method->name.lexeme +
                    "' cannot have the same name as another method.");
        }

        methods.insert(std::pair{method->name.lexeme, getFunctionType(*method)});
    }

    m_types.insert(std::make_pair(stmt.name.lexeme, std::make_shared<TraitType>(stmt.name.lexeme, methods)));
}

void Analyser::visitWhileStmt(WhileStmt& stmt) {
    analyse(stmt.condition);

    m_insideLoop = true;
    beginScope();
    for (Stmt& statement : stmt.body) {
        analyse(statement);
    }
    endScope();
    m_insideLoop = false;
}

void Analyser::visitVariableStmt(VariableStmt& stmt) {
    analyse(stmt.initializer);

    std::string typeName = stmt.typeName;

    if (stmt.typeName.empty()) {
        // Infer the type from the initializer
        typeName = stmt.initializer->getType()->toString();
    }

    if (!stmt.initializer->getType()->looselyEquals(*lookUpType(typeName, stmt.name))) {
        throw errorAt(stmt.name, "Cannot initialize variable of type '" + typeName +
                "' with initializer of type '" + stmt.initializer->getType()->toString() + "'.");
    }

    declareVariable(stmt.name.lexeme, Variable{lookUpType(typeName, stmt.name), stmt.isConst});
}

void Analyser::visitAnyExpr(AnyExpr& expr) {
    throw AnalysisError{};
}

void Analyser::visitArrayExpr(ArrayExpr& expr) {
    std::vector<Type> elementTypes{};
    for (Expr& element : expr.value) {
        analyse(element);
        elementTypes.push_back(element->getType());
    }

    if (!expr.typeName.empty()) {
        Type elementType = lookUpType(expr.typeName, expr.square);

        for (Type& type : elementTypes) {
            if (!elementType->looselyEquals(*type)) {
                throw errorAt(expr.square, "Array literal of specified type '" + elementType->toString() +
                        "' cannot contain an element of type '" + type->toString() + "'.");
            }
        }

        expr.setType(std::make_shared<ArrayType>(elementType));
    } else {
        Type elementType = (elementTypes.empty() ? m_types["any"] : elementTypes[0]);

        for (int i = 1; i < elementTypes.size(); ++i) {
            if (*elementTypes[i] != *elementTypes[i-1]) {
                expr.setType(std::make_shared<ArrayType>(m_types["any"]));
                break;
            }

            elementType = elementTypes[i];
        }

        expr.setType(std::make_shared<ArrayType>(elementType));
    }
}

void Analyser::visitAssignExpr(AssignExpr& expr) {
    analyse(expr.left);
    analyse(expr.right);

    if (!expr.left->getType()->looselyEquals(*expr.right->getType())) {
        throw errorAt(expr.oper, "Cannot assign variable of type '" + expr.left->getType()->toString() +
                "' with initializer of type '" + expr.right->getType()->toString() + "'.");
    }
}

void Analyser::visitBinaryExpr(BinaryExpr& expr) {
    analyse(expr.left);
    analyse(expr.right);

    Type left = expr.left->getType();
    Type right = expr.right->getType();

    switch (expr.oper.type) {
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
            if (!left->maybeNumeric() ||
                    !expr.right->getType()->maybeNumeric()) {
                throw errorAt(expr.oper, "Operator '" + expr.oper.lexeme + "' may only be applied to numbers.");
            }

            if (left->isFloat() || right->isFloat()) {
                expr.setType(m_types["float"]);
            } else if (left->isInt() && right->isInt()) {
                expr.setType(m_types["int"]);
            } else {
                expr.setType(m_types["any"]);
            }

            break;

        case TokenType::PLUS:
            if (!(expr.left->getType()->maybeNumeric() &&
                    expr.right->getType()->maybeNumeric()) &&
                    !(expr.left->getType()->maybeString() &&
                    expr.right->getType()->maybeString())) {
                throw errorAt(expr.oper, "Can only add two numbers or two strings.");
            }

            if (left->isString() && right->isString()) {
                expr.setType(m_types["string"]);
            } else if (left->isFloat() || right->isFloat()) {
                expr.setType(m_types["float"]);
            } else if (left->isInt() && right->isInt()) {
                expr.setType(m_types["int"]);
            } else {
                expr.setType(m_types["any"]);
            }

            break;

        default: throw errorAt(expr.oper, "Unreachable.");
    }
}

void Analyser::visitBooleanExpr(BooleanExpr& expr) {
    expr.setType(m_types["bool"]);
}

void Analyser::visitCallExpr(CallExpr& expr) {
    analyse(expr.callee);

    Type type = expr.callee->getType();

    if (type->isConstructor()) {
        type = std::make_shared<FunctionType>(type->as<ConstructorType>()->getFunctionType());
    }

    if (type->isFunction()) {
        auto calleeType = type->as<FunctionType>();

        // Do we have the correct amount of arguments?
        if (expr.arguments.size() != calleeType->getArgumentTypes().size()) {
            throw errorAt(expr.paren, "Expected " + std::to_string(calleeType->getArgumentTypes().size()) +
                    " arguments, but got " + std::to_string(expr.arguments.size()) + ".");
        }

        // Are the arguments we have the right type?
        for (int i = 0; i < expr.arguments.size(); ++i) {
            analyse(expr.arguments[i]);
            if (!expr.arguments[i]->getType()->looselyEquals(*calleeType->getArgumentTypes()[i])) {
                throw errorAt(expr.paren, "Expected argument of type '" + calleeType->getArgumentTypes()[i]->toString() +
                        "' but got '" + expr.arguments[i]->getType()->toString() + "'.");
            }
        }

        // The call expression's type is the return type of the callee.
        expr.setType(calleeType->getReturnType());
    } else if (type->isDynamic()) {
        // We'll have to check this one at runtime.
        expr.setType(std::make_shared<PrimitiveType>(PrimitiveKind::DYNAMIC));
    } else {
        throw errorAt(expr.paren, "Only functions can be called.");
    }
}

void Analyser::visitFieldExpr(FieldExpr& expr) {
    analyse(expr.object);

    Type objectType = expr.object->getType();

    if (objectType->isStruct()) {
        auto type = objectType->as<StructType>();

        if (auto t = type->getFieldOrMethod(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.oper, "Struct type '" + type->getName() + "' does not have a field or method named '" +
                                 expr.name.lexeme + "'.");
    } else if (objectType->isTrait()) {
        auto type = objectType->as<TraitType>();

        if (auto t = type->getMethod(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.oper, "Trait type '" + type->getName() + "' does not have a method named '" +
                                 expr.name.lexeme + "'.");

    } else if (objectType->isConstructor()) {
        auto type = objectType->as<ConstructorType>()->getStructType();

        if (auto t = type.getAssocFunction(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.oper, "Struct type '" + type.getName() + "' does not have an associated function named '" +
                                 expr.name.lexeme + "'.");
    } else if (objectType->isDynamic()) {
        // We'll have to check at runtime.
        expr.setType(objectType);
    } else {
        throw errorAt(expr.oper, "Only structs, traits and types have fields.");
    }
}

void Analyser::visitFloatExpr(FloatExpr& expr) {
    expr.setType(m_types["float"]);
}

void Analyser::visitIntegerExpr(IntegerExpr& expr) {
    expr.setType(m_types["int"]);
}

void Analyser::visitLogicalExpr(LogicalExpr& expr) {

}

void Analyser::visitNilExpr(NilExpr& expr) {
    expr.setType(m_types["nothing"]);
}

void Analyser::visitReferenceExpr(ReferenceExpr& expr) {
    throw errorAt(expr.oper, "Unsupported feature.");
}

void Analyser::visitStringExpr(StringExpr& expr) {
    expr.setType(m_types["string"]);
}

void Analyser::visitSubscriptExpr(SubscriptExpr& expr) {

}

void Analyser::visitTernaryExpr(TernaryExpr& expr) {

}

void Analyser::visitUnaryExpr(UnaryExpr& expr) {
    analyse(expr.operand);
    switch (expr.oper.type) {
        case TokenType::BANG:
            expr.setType(m_types["bool"]);
            break;

        case TokenType::MINUS:
            if (!(expr.operand->getType()->maybeNumeric())) {
                throw errorAt(expr.oper, "Only numbers can be negated.");
            }
            expr.setType(expr.operand->getType());
            break;

        default: break; // Unreachable.
    }
}

void Analyser::visitVariableExpr(VariableExpr& expr) {
    expr.setType(lookUpVariable(expr.name).type);
}

Type Analyser::getFunctionType(const FunctionStmt &stmt) {
    Type returnType = lookUpType(stmt.returnTypeName, stmt.name);

    std::vector<Type> parameterTypes;
    for (const NamedTypename& parameter : stmt.params) {
        parameterTypes.push_back(lookUpType(parameter.typeName, parameter.name));
    }

    return std::make_shared<FunctionType>(returnType, parameterTypes);
}

Type Analyser::lookUpType(const Token &name) {
    lookUpType(name.lexeme, name);
}

Type Analyser::lookUpType(const std::string &name, const Token &where) {
    if (!name.empty()) {
        if (name[0] == '[' && name[name.size()-1] == ']') {
            std::string elementTypeName = name.substr(1, name.size()-2);
            return std::make_shared<ArrayType>(lookUpType(elementTypeName, where));
        }
    }

    if (m_types.count(name) > 0) {
        return m_types[name];
    }

    throw errorAt(where, "Undefined type '" + name + "'.");
}

Analyser::Variable& Analyser::lookUpVariable(const Token& name) {
    for (auto scope = m_scopes.rbegin(); scope != m_scopes.rend(); ++scope) {
        if (scope->count(name.lexeme) > 0) {
            return (*scope)[name.lexeme];
        }
    }

    throw errorAt(name, "Undefined variable '" + name.lexeme + "'.");
}

void Analyser::declareVariable(const std::string &name, const Analyser::Variable &variable) {
    auto& scope = m_scopes[m_scopes.size()-1];

    if (scope.count(name) > 0) {
        scope[name] = variable;
    } else {
        scope.insert(std::make_pair(name, variable));
    }
}

void Analyser::beginScope() {
    m_scopes.emplace_back(std::unordered_map<std::string, Variable>());
}

void Analyser::endScope() {
    m_scopes.pop_back();
}