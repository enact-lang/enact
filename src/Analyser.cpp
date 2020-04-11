#include <vector>
#include <set>
#include <sstream>
#include "h/Analyser.h"
#include "h/Context.h"

Analyser::Analyser(Context& context) : m_context{context} {
}

std::vector<std::unique_ptr<Stmt>> Analyser::analyse(std::vector<std::unique_ptr<Stmt>> ast) {
    m_hadError = false;

    if (m_scopes.empty()) {
        beginScope();

        declareVariable("", Variable{std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{}), true});
        declareVariable("print",
                        Variable{std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{DYNAMIC_TYPE}, false, true), true});
        declareVariable("put",
                        Variable{std::make_shared<FunctionType>(NOTHING_TYPE, std::vector<Type>{DYNAMIC_TYPE}, false, true), true});
        declareVariable("dis",
                        Variable{std::make_shared<FunctionType>(STRING_TYPE, std::vector<Type>{DYNAMIC_TYPE}, false, true), true});
    }

    for (auto &stmt : ast) {
        analyse(*stmt);
    }

    // Analyse all function bodies in the global scope
    for (auto it = m_globalFunctions.rbegin(); it != m_globalFunctions.rend(); it++) {
        analyseFunctionBody(*it);
        m_globalFunctions.pop_back();
    }

    return ast;
}

void Analyser::analyse(Stmt& stmt) {
    try {
        stmt.accept(this);
    } catch (const AnalysisError &error) {
        m_hadError = true;
    }
}

void Analyser::analyse(Expr& expr) {
    expr.accept(this);
}

bool Analyser::hadError() {
    return m_hadError;
}

Analyser::AnalysisError Analyser::errorAt(const Token &token, const std::string &message) {
    m_context.reportErrorAt(token, message);
    m_hadError = true;
    return AnalysisError{};
}

void Analyser::visitBlockStmt(BlockStmt &stmt) {
    beginScope();
    for (auto& statement : stmt.statements) {
        analyse(*statement);
    }
    endScope();
}

void Analyser::visitBreakStmt(BreakStmt &stmt) {
    if (m_loopCount == 0) {
        throw errorAt(stmt.keyword, "Break is only allowed inside loops.");
    }
}

void Analyser::visitContinueStmt(ContinueStmt &stmt) {
    if (m_loopCount == 0) {
        throw errorAt(stmt.keyword, "Continue is only allowed inside loops.");
    }
}

void Analyser::visitEachStmt(EachStmt &stmt) {
    throw errorAt(stmt.name, "Currently unsupported! Waiting for implementation of generics.");
}

void Analyser::visitExpressionStmt(ExpressionStmt &stmt) {
    analyse(*stmt.expr);
}

void Analyser::visitForStmt(ForStmt &stmt) {
    beginScope();
    analyse(*stmt.initializer);

    analyse(*stmt.condition);
    if (!stmt.condition->getType()->maybeBool()) {
        throw errorAt(stmt.keyword, "Expected for loop condition of type 'bool', not '"
                + stmt.condition->getType()->toString() + "'.");
    }

    m_loopCount++;
    beginScope();
    for (auto& statement : stmt.body) {
        analyse(*statement);
    }
    endScope();
    m_loopCount--;

    analyse(*stmt.increment);
    endScope();
}

void Analyser::visitFunctionStmt(FunctionStmt &stmt) {
    stmt.type = getFunctionType(stmt);

    declareVariable(stmt.name.lexeme, Variable{stmt.type, true});

    if (m_scopes.size() == 1) {
        m_globalFunctions.emplace_back(stmt);
    } else {
        analyseFunctionBody(stmt);
    }
}

void Analyser::visitGivenStmt(GivenStmt &stmt) {
    analyse(*stmt.value);
    Type valueType = stmt.value->getType();

    for (const GivenCase &case_ : stmt.cases) {
        analyse(*case_.value);
        if (!case_.value->getType()->looselyEquals(*valueType)) {
            throw errorAt(case_.keyword, "Given value of type '" + valueType->toString()
                                         + "' cannot be compared with case of type '" +
                                         case_.value->getType()->toString() + "'.");
        }

        beginScope();

        for (auto& bodyStmt : case_.body) {
            analyse(*bodyStmt);
        }

        endScope();
    }
}

void Analyser::visitIfStmt(IfStmt &stmt) {
    analyse(*stmt.condition);

    if (!stmt.condition->getType()->maybeBool()) {
        throw errorAt(stmt.keyword, "Expected if statement condition of type 'bool', not '"
                + stmt.condition->getType()->toString() + "'.");
    }

    beginScope();
    for (auto& statement : stmt.thenBlock) {
        analyse(*statement);
    }
    endScope();

    beginScope();
    for (auto& statement : stmt.elseBlock) {
        analyse(*statement);
    }
    endScope();
}

void Analyser::visitReturnStmt(ReturnStmt &stmt) {
    if (m_currentFunctions.empty()) {
        throw errorAt(stmt.keyword, "Return is only allowed inside functions.");
    }

    analyse(*stmt.value);

    Type returnType = m_currentFunctions.back().getReturnType();
    if (!returnType->looselyEquals(*stmt.value->getType())) {
        throw errorAt(stmt.keyword, "Cannot return from function with return type '" +
                                    returnType->toString() + "' with value of type '" +
                                    stmt.value->getType()->toString() + "'.");
    }
}

void Analyser::visitStructStmt(StructStmt &stmt) {
    if (m_types.count(stmt.name.lexeme) > 0) {
        throw errorAt(stmt.name, "Cannot redeclare type '" + stmt.name.lexeme + "'.");
    }

    m_types.emplace(stmt.name.lexeme, nullptr);

    std::vector<std::shared_ptr<const TraitType>> traits;
    for (const Token &traitName : stmt.traits) {
        // Check that the trait has been declared as a type.
        if (m_types.count(traitName.lexeme) > 0) {
            // Check that the trait actually is a trait, and not an 'int' or something.
            if (m_types[traitName.lexeme]->isTrait()) {
                traits.push_back(std::shared_ptr<const TraitType>{
                    m_types[traitName.lexeme]->as<TraitType>()
                });
            } else {
                throw errorAt(traitName, "Type '" + traitName.lexeme + "' is not a trait.");
            }
        } else {
            throw errorAt(traitName, "Undeclared trait '" + traitName.lexeme + "'.");
        }
    }

    // In the AST, fields are represented as a name paired with a type (Field).
    // We must now find the types that the typenames are referring to, keeping
    // track of them in an insertion order map to make things easier for the
    // Compiler.
    InsertionOrderMap<std::string, Type> fields{};
    for (const Field& field : stmt.fields) {
        // Check if the field has the same name as another field
        if (fields.count(field.name.lexeme) > 0) {
            throw errorAt(field.name, "Struct field '" + field.name.lexeme +
                                      "' cannot have the same name as another field.");
        }

        if (m_types.count(field.typeName->name()) > 0) {
            fields.insert(std::pair(field.name.lexeme, m_types[field.typeName->name()]));
        } else {
            throw errorAt(field.name, "Undeclared type '" + field.typeName->name() + "' of field.");
        }
    }

    // Methods aren't just fields with a function value, as they are bound to an object (self).
    // As such, they need to be treated differently from fields and kept separate.
    InsertionOrderMap<std::string, Type> methods;
    for (auto& method : stmt.methods) {
        // Check if the method has the same name as a field
        if (fields.contains(method->name.lexeme) || methods.contains(method->name.lexeme)) {
            throw errorAt(method->name, "Struct method '" + method->name.lexeme +
                                        "' cannot have the same name as another field or method.");
        }

        methods.insert(std::pair(method->name.lexeme, getFunctionType(*method, true)));
    }

    // Check that the traits are satisfied now
    for (const auto& traitType: traits) {
        // Check methods have been defined
        for (const auto& requiredMethod : traitType->getMethods()) {
            bool found = false;

            for (const auto& method : methods) {
                if (method.first == requiredMethod.first
                        && method.second->looselyEquals(*requiredMethod.second)) {
                    found = true;
                    break;
                }
            }

            if (found) continue;

            throw errorAt(stmt.name, "Method '" + requiredMethod.first + "' is required by trait '"
                + traitType->toString() + "' but is not implemented.");
        }
    }

    // Assoc functions must again be kept separate from the other kinds of properties,
    // as they are called on the type rather than an instance of the type.
    InsertionOrderMap<std::string, Type> assocFunctions;
    for (auto& function : stmt.assocFunctions) {
        assocFunctions.insert(std::pair(function->name.lexeme, getFunctionType(*function)));
    }

    auto thisType = std::make_shared<StructType>(stmt.name.lexeme, traits, fields, methods);
    m_types[stmt.name.lexeme] = thisType;

    for (size_t i = 0; i < stmt.methods.size(); ++i) {
        methods.atIndex(i)->get() = getFunctionType(*stmt.methods[i], true);
    }

    for (size_t i = 0; i < stmt.assocFunctions.size(); ++i) {
        assocFunctions.atIndex(i)->get() = getFunctionType(*stmt.assocFunctions[i]);
    }

    // Now, create a constructor for the struct.
    auto constructorType = std::make_shared<const ConstructorType>(thisType, assocFunctions);
    stmt.constructorType = constructorType;

    declareVariable(stmt.name.lexeme, Variable{constructorType, true});

    // Now we can analyse the methods:
    for (auto& method : stmt.methods) {
        method->type = getFunctionType(*method, true);
        beginScope();

        declareVariable("self", Variable{thisType, !method->isMut});
        analyse(*method);

        endScope();
    }

    // Look at the assoc functions (remember, they are outside of the struct scope):
    for (auto& function : stmt.assocFunctions) {
        function->type = getFunctionType(*function);
        analyse(*function);
    }
}

void Analyser::visitTraitStmt(TraitStmt &stmt) {
    if (m_types.count(stmt.name.lexeme) > 0) {
        throw errorAt(stmt.name, "Cannot redeclare type '" + stmt.name.lexeme + "'.");
    }

    InsertionOrderMap<std::string, Type> methods;
    for (auto& method : stmt.methods) {
        if (methods.contains(method->name.lexeme)) {
            throw errorAt(method->name, "Trait method '" + method->name.lexeme +
                                        "' cannot have the same name as another method.");
        }

        methods.insert(std::pair{method->name.lexeme, getFunctionType(*method)});
    }

    m_types.insert(std::make_pair(stmt.name.lexeme, std::make_shared<TraitType>(stmt.name.lexeme, methods)));
}

void Analyser::visitWhileStmt(WhileStmt &stmt) {
    analyse(*stmt.condition);
  
    if (!stmt.condition->getType()->maybeBool()) {
        throw errorAt(stmt.keyword, "Expected while loop condition of type 'bool', not '"
                + stmt.condition->getType()->toString() + "'.");
    }

    m_loopCount++;
    beginScope();
    for (auto& statement : stmt.body) {
        analyse(*statement);
    }
    endScope();
    m_loopCount--;
}

void Analyser::visitVariableStmt(VariableStmt &stmt) {
    analyse(*stmt.initializer);

    std::unique_ptr<const Typename> typeName = stmt.typeName->clone();

    if (typeName->name().empty()) {
        // Infer the type from the initializer
        typeName = stmt.initializer->getType()->toTypename();
    }

    if (!stmt.initializer->getType()->looselyEquals(*lookUpType(*typeName))) {
        throw errorAt(stmt.name, "Cannot initialize variable of type '" + typeName->name() +
                                 "' with value of type '" + stmt.initializer->getType()->toString() + "'.");
    }

    declareVariable(stmt.name.lexeme, Variable{lookUpType(*typeName), stmt.isConst});
}

void Analyser::visitAllotExpr(AllotExpr& expr) {
    analyse(*expr.target);
    analyse(*expr.value);

    if (!expr.target->getType()->looselyEquals(*expr.value->getType())) {
        throw errorAt(expr.oper, "Cannot assign variable of type '" + expr.target->getType()->toString() +
                                 "' with value of type '" + expr.value->getType()->toString() + "'.");
    }
}

void Analyser::visitAnyExpr(AnyExpr &expr) {
    throw AnalysisError{};
}

void Analyser::visitArrayExpr(ArrayExpr &expr) {
    std::vector<Type> elementTypes{};
    for (auto& element : expr.value) {
        analyse(*element);
        elementTypes.push_back(element->getType());
    }

    if (!expr.typeName->name().empty()) {
        Type elementType = lookUpType(*expr.typeName);

        for (Type &type : elementTypes) {
            if (!elementType->looselyEquals(*type)) {
                throw errorAt(expr.square, "Array literal of specified type '" + elementType->toString() +
                                           "' cannot contain an element of type '" + type->toString() + "'.");
            }
        }

        expr.setType(std::make_shared<ArrayType>(elementType));
    } else {
        Type elementType = (elementTypes.empty() ? m_types["any"] : elementTypes[0]);

        for (int i = 1; i < elementTypes.size(); ++i) {
            if (*elementTypes[i] != *elementTypes[i - 1]) {
                expr.setType(std::make_shared<ArrayType>(m_types["any"]));
                break;
            }

            elementType = elementTypes[i];
        }

        expr.setType(std::make_shared<ArrayType>(elementType));
    }
}

void Analyser::visitAssignExpr(AssignExpr &expr) {
    analyse(*expr.target);
    analyse(*expr.value);

    auto& name = expr.target->name;
    if (lookUpVariable(name).isConst) {
        throw errorAt(name, "Cannot assign to constant variable.");
    }

    if (!expr.target->getType()->looselyEquals(*expr.value->getType())) {
        throw errorAt(expr.oper, "Cannot assign variable of type '" + expr.target->getType()->toString() +
                                 "' with value of type '" + expr.value->getType()->toString() + "'.");
    }
}

void Analyser::visitBinaryExpr(BinaryExpr &expr) {
    analyse(*expr.left);
    analyse(*expr.right);

    Type left = expr.left->getType();
    Type right = expr.right->getType();

    switch (expr.oper.type) {
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
            if (!left->maybeNumeric() ||
                !right->maybeNumeric()) {
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

        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
            if (!left->maybeNumeric() ||
                !right->maybeNumeric()) {
                throw errorAt(expr.oper, "Operator '" + expr.oper.lexeme + "' may only be applied to numbers.");
            }

            expr.setType(m_types["bool"]);
            break;

        case TokenType::PLUS:
            if (!((left->maybeNumeric() &&
                   right->maybeNumeric()) ||
                  (left->maybeString() &&
                   right->maybeString()))) {
                throw errorAt(expr.oper, "Can only add two numbers or two strings.");
            }

            if (left->isString() && right->isString()) {
                expr.setType(m_types["String"]);
            } else if (left->isFloat() || right->isFloat()) {
                expr.setType(m_types["float"]);
            } else if (left->isInt() && right->isInt()) {
                expr.setType(m_types["int"]);
            } else {
                expr.setType(m_types["any"]);
            }

            break;

        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            if (!left->looselyEquals(*right)) {
                throw errorAt(expr.oper, "Cannot check for equality between mismatched types '"
                                         + left->toString() + "' and '" + right->toString() + "'.");
            }

            expr.setType(m_types["bool"]);
            break;

        default:
            throw errorAt(expr.oper, "Unreachable.");
    }
}

void Analyser::visitBooleanExpr(BooleanExpr &expr) {
    expr.setType(m_types["bool"]);
}

void Analyser::visitCallExpr(CallExpr &expr) {
    analyse(*expr.callee);
    for (auto& argument : expr.arguments) {
        analyse(*argument);
    }

    Type calleeType = expr.callee->getType();

    if (calleeType->isDynamic()) {
        // We'll have to check this one at runtime.
        expr.setType(DYNAMIC_TYPE);
        return;
    }

    Type returnType;
    std::vector<Type> paramTypes;

    if (calleeType->isConstructor()) {
        auto constructorType = calleeType->as<ConstructorType>();

        returnType = constructorType->getStructType();
        for (const auto& property : constructorType->getStructType()->getFields()) {
            paramTypes.push_back(property.second);
        }
    } else if (calleeType->isFunction()) {
        auto functionType = calleeType->as<FunctionType>();

        returnType = functionType->getReturnType();
        paramTypes = functionType->getArgumentTypes();
    } else {
        throw errorAt(expr.paren, "Only functions or constructors can be called.");
    }

    // Do we have the correct amount of arguments?
    if (expr.arguments.size() != paramTypes.size()) {
        throw errorAt(expr.paren, "Expected " + std::to_string(paramTypes.size()) +
                " arguments, but got " + std::to_string(expr.arguments.size()) + ".");
    }

    // Are the arguments we have the right type?
    for (int i = 0; i < expr.arguments.size(); ++i) {
        if (!expr.arguments[i]->getType()->looselyEquals(*paramTypes[i])) {
            throw errorAt(expr.paren, "Expected argument of type '" + paramTypes[i]->toString() +
                    "' but got '" + expr.arguments[i]->getType()->toString() + "'.");
        }
    }

    expr.setType(returnType);
}

void Analyser::visitFloatExpr(FloatExpr &expr) {
    expr.setType(m_types["float"]);
}

void Analyser::visitGetExpr(GetExpr &expr) {
    analyse(*expr.object);

    Type objectType = expr.object->getType();

    if (objectType->isStruct()) {
        auto type = objectType->as<StructType>();

        if (auto t = type->getProperty(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.name, "Struct type '" + type->getName() + "' does not have a property named '" +
                                 expr.name.lexeme + "'.");
    } else if (objectType->isTrait()) {
        auto type = objectType->as<TraitType>();

        if (auto t = type->getMethod(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.name, "Trait type '" + type->getName() + "' does not have a method named '" +
                                 expr.name.lexeme + "'.");

    } else if (objectType->isConstructor()) {
        auto type = objectType->as<ConstructorType>();

        if (auto t = type->getAssocProperty(expr.name.lexeme)) {
            expr.setType(*t);
            return;
        }

        throw errorAt(expr.name, "Struct type '" + type->getStructType()->getName() + "' does not have an associated function named '" +
                                 expr.name.lexeme + "'.");
    } else if (objectType->isDynamic()) {
        // We'll have to check at runtime.
        expr.setType(objectType);
    } else {
        throw errorAt(expr.name, "Only structs, traits and types have fields.");
    }
}

void Analyser::visitIntegerExpr(IntegerExpr &expr) {
    expr.setType(m_types["int"]);
}

void Analyser::visitLogicalExpr(LogicalExpr &expr) {
    analyse(*expr.left);
    analyse(*expr.right);

    Type left = expr.left->getType();
    Type right = expr.right->getType();

    if (!left->maybeBool() &&
        !right->maybeBool()) {
        throw errorAt(expr.oper, "Operator '" + expr.oper.lexeme + "' may only be applied to booleans.");
    }

    expr.setType(m_types["bool"]);
}

void Analyser::visitNilExpr(NilExpr &expr) {
    expr.setType(m_types["nothing"]);
}

void Analyser::visitSetExpr(SetExpr& expr) {
    analyse(*expr.target);
    analyse(*expr.value);

    Type objectType = expr.target->object->getType();
    std::string name = expr.target->name.lexeme;

    if (objectType->isStruct()
            && objectType->as<StructType>()->findMethod(name)) {
        throw errorAt(expr.oper, "Cannot assign to a struct method.");
    }

    if (objectType->isConstructor()
            && objectType->as<ConstructorType>()->findAssocProperty(name)) {
        throw errorAt(expr.oper, "Cannot assign to an assoc method.");
    }

    if (!expr.target->getType()->looselyEquals(*expr.value->getType())) {
        throw errorAt(expr.oper, "Cannot assign value of type '" + expr.value->getType()->toString()
                + "' to property '" + expr.target->name.lexeme + "' of type '" + expr.value->getType()->toString()
                + "'.");
    }

    expr.setType(expr.value->getType());
}

void Analyser::visitStringExpr(StringExpr &expr) {
    expr.setType(m_types["String"]);
}

void Analyser::visitSubscriptExpr(SubscriptExpr &expr) {
    analyse(*expr.object);
    analyse(*expr.index);

    if (!expr.object->getType()->maybeArray()) {
        throw errorAt(expr.square, "Only arrays can be subscripted.");
    }

    if (!expr.index->getType()->maybeInt()) {
        throw errorAt(expr.square, "Subscript index must be an integer.");
    }

    if (expr.object->getType()->isDynamic()) {
        expr.setType(m_types["any"]);
        return;
    }

    expr.setType(expr.object->getType()->as<ArrayType>()->getElementType());
}

void Analyser::visitTernaryExpr(TernaryExpr &expr) {
    analyse(*expr.condition);
    if (!expr.condition->getType()->maybeBool()) {
        throw errorAt(expr.oper, "Conditional operator condition must be a boolean.");
    }

    analyse(*expr.thenExpr);
    analyse(*expr.elseExpr);

    if (!expr.thenExpr->getType()->looselyEquals(*expr.elseExpr->getType())) {
        throw errorAt(expr.oper, "Mismatched types in conditional operation: '" + expr.thenExpr->getType()->toString()
                                 + "' and '" + expr.elseExpr->getType()->toString() + "'.");
    }
}

void Analyser::visitUnaryExpr(UnaryExpr &expr) {
    analyse(*expr.operand);
    switch (expr.oper.type) {
        case TokenType::BANG:
            if (!(expr.operand->getType()->maybeBool())) {
                throw errorAt(expr.oper, "Only booleans can be inverted.");
            }
            expr.setType(expr.operand->getType());
            break;

        case TokenType::MINUS:
            if (!(expr.operand->getType()->maybeNumeric())) {
                throw errorAt(expr.oper, "Only numbers can be negated.");
            }
            expr.setType(expr.operand->getType());
            break;

        case TokenType::COPY:
            if (expr.operand->getType()->isPrimitive()) {
                throw errorAt(expr.oper, "Cannot copy the value type '" +
                        expr.operand->getType()->toString() + "'.");
            }
            expr.setType(expr.operand->getType());
            break;

        default:
            break; // Unreachable.
    }
}

void Analyser::visitVariableExpr(VariableExpr &expr) {
    expr.setType(lookUpVariable(expr.name).type);
}

void Analyser::analyseFunctionBody(FunctionStmt &stmt) {
    Type type = getFunctionType(stmt);
    auto functionType = type->as<FunctionType>();

    beginScope();
    m_currentFunctions.push_back(*functionType);

    for (int i = 0; i < stmt.params.size(); ++i) {
        declareVariable(stmt.params[i].name.lexeme, Variable{functionType->getArgumentTypes()[i]});
    }

    for (auto& statement : stmt.body) {
        analyse(*statement);
    }

    m_currentFunctions.pop_back();
    endScope();
}

Type Analyser::getFunctionType(const FunctionStmt &stmt, bool isMethod, bool isNative) {
    Type returnType;
    if (stmt.returnTypename->name().empty()) {
        returnType = NOTHING_TYPE;
    } else {
        returnType = lookUpType(*stmt.returnTypename);
    }

    if (stmt.params.size() > 255) {
        throw errorAt(stmt.name, "Cannot have more than 255 arguments to a function.");
    }

    std::vector<Type> parameterTypes;
    for (const Param& parameter : stmt.params) {
        parameterTypes.push_back(lookUpType(*parameter.typeName));
    }

    return std::make_shared<FunctionType>(returnType, parameterTypes, isMethod, isNative);
}

Type Analyser::lookUpType(const Typename& name) {
    switch (name.kind()) {
        case Typename::Kind::BASIC:
            if (m_types.count(name.name()) > 0) {
                return m_types[name.name()];
            } else if (name.name() == "") {
                return m_types["nothing"];
            }
            break;
        case Typename::Kind::ARRAY: {
            const auto& arrName = static_cast<const ArrayTypename&>(name);
            return std::make_shared<ArrayType>(lookUpType(arrName.elementTypename()));
        }
        case Typename::Kind::FUNCTION: {
            const auto& funName = static_cast<const FunctionTypename&>(name);

            std::vector<Type> argTypes{};
            for (const auto& argName : funName.argumentTypenames()) {
                argTypes.push_back(lookUpType(*argName));
            }

            return std::make_shared<FunctionType>(lookUpType(funName.returnTypename()), std::move(argTypes));
        }
        case Typename::Kind::CONSTRUCTOR: {
            const auto& conName = static_cast<const ConstructorTypename&>(name);
            return lookUpVariable(conName.structTypename().where()).type;
        }
    }

    throw errorAt(name.where(), "Undefined type '" + name.name() + "'.");
}

Analyser::Variable &Analyser::lookUpVariable(const Token &name) {
    for (auto scope = m_scopes.rbegin(); scope != m_scopes.rend(); ++scope) {
        if (scope->count(name.lexeme) > 0) {
            return (*scope)[name.lexeme];
        }
    }

    throw errorAt(name, "Undefined variable '" + name.lexeme + "'.");
}

void Analyser::declareVariable(const std::string &name, const Analyser::Variable &variable) {
    auto &scope = m_scopes[m_scopes.size() - 1];

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
