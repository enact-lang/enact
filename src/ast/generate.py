# THIS SCRIPT IS DEPRECATED
# It will NOT generate a correct AST

# This script generates the AST classes found in Expr.h/cpp and Stmt.h/cpp.


def uncapitalize(s):
    s[:1].lower() + s[1:] if s else ''


def generate_ast_visitor(name, type_fields, visitor_types):
    ret = f"template <class R>\nclass {name}Visitor {{\npublic:\n"
    for key in type_fields:
        ret += f"    virtual R visit{key}{name}({key}{name}& {name.lower()}) = 0;\n"
    ret += "};\n\n"
    return ret


def generate_ast_class_visitors(name, type_fields, visitor_types):
    ret = ""
    for type in visitor_types:
        ret += f"    virtual {type} accept({name}Visitor<{type}> *visitor) = 0;\n"
    return ret


def generate_ast_subclasses(name, type_fields, visitor_types):
    ret = ""
    for key in type_fields:
        ret += f"class {key}{name} : public {name}Base {{\npublic:\n"
        if len(type_fields[key]) > 0:
            for field in type_fields[key]:
                ret += "    " + field + ";\n"
            ret += f"\n    {key}{name}("
            for field in type_fields[key]:
                ret += field + ","
            ret = ret[:len(ret)-1]
            ret += ") : \n        "
            for field in type_fields[key]:
                ret += f"{field.split(' ')[1]}{{{field.split(' ')[1]}}},"
            ret = ret[:len(ret)-1]
            ret += " {}\n"
        else:
            ret += f"    {key}{name}() = default;\n"

        ret += f"    ~{key}{name}() override = default;\n"

        for visitor_type in visitor_types:
            ret += f"\n    {visitor_type} accept({name}Visitor<{visitor_type}> *visitor) override {{\n        return visitor->visit{key+name}(*this);\n    }}\n"
        ret += "};\n\n"
    return ret


def generate_ast_subclass_decls(name, type_fields):
    ret = ""
    for type_field in type_fields:
        ret += f"class {type_field}{name};\n"
    ret += "\n"
    return ret


def generate_ast_class_body(name, type_fields, visitor_types):
    ret = f"    virtual ~{name}Base() = default;\n"
    ret += "\n" + generate_ast_class_visitors(name, type_fields, visitor_types)
    ret += "};\n\n"
    return ret


def generate_includes(includes):
    ret = ""
    for include in includes:
        ret += f"#include {include}\n"
    return ret


def generate_ast_class(name, type_fields, visitor_types, includes):
    type_field = ""
    type_getter = ""

    if name == "Expr":
        type_field = "    Type m_type = nullptr;\n"
        type_getter = "    virtual void setType(Type t) { m_type = t; }\n    virtual const Type& getType() {\n        ENACT_ASSERT(m_type != nullptr, \"Expr::getType(): Tried to get uninitialized type.\");\n        return m_type;\n    }\n"

    ret = ("// This file was automatically generated.\n"
           "// \"See generate.py\" for details.\n\n"
           f"#ifndef ENACT_{name.upper()}_H\n"
           f"#define ENACT_{name.upper()}_H\n\n"

           f"{generate_includes(includes)}\n"
           
           f"template <class R>\nclass {name}Visitor;\n\n"
           
           f"class {name}Base {{\n"
           
           f"{type_field}"
           
           "public:\n"
           
           f"{type_getter}"

           f"{generate_ast_class_body(name, type_fields, visitor_types)}"
           
           f"typedef std::shared_ptr<{name}Base> {name};\n\n"

           f"{generate_ast_subclass_decls(name, type_fields)}"
           f"{generate_ast_visitor(name, type_fields, visitor_types)}"
           
           f"{generate_ast_subclasses(name, type_fields, visitor_types)}"
           
           f"#endif // ENACT_{name.upper()}_H\n"
           )
    return ret


def generate_tree(name, type_fields, visitor_types, includes):
    with open(name + ".h", "w") as file:
        file.write(generate_ast_class(name, type_fields, visitor_types, includes))


generate_tree(
    "Expr",
    {
        "Allot":    ["std::shared_ptr<SubscriptExpr> target", "Expr value", "Token oper"],
        "Any":      [],
        "Array":    ["std::vector<Expr> value", "Token square", "std::unique_ptr<Typename> typeName"],
        "Assign":   ["std::shared_ptr<VariableExpr> target", "Expr value", "Token oper"],
        "Binary":   ["Expr left", "Expr right", "Token oper"],
        "Boolean":  ["bool value"],
        "Call":     ["Expr callee", "std::vector<Expr> arguments", "Token paren"],
        "Float":    ["double value"],
        "Get":      ["Expr object", "Token name", "Token oper"],
        "Integer":  ["int value"],
        "Logical":  ["Expr left", "Expr right", "Token oper"],
        "Nil":      [],
        "String":   ["std::string value"],
        "Subscript":["Expr object", "Expr index", "Token square"],
        "Ternary":  ["Expr condition", "Expr thenExpr", "Expr elseExpr", "Token oper"],
        "Unary":    ["Expr operand", "Token oper"],
        "Variable": ["Token name"]
    },
    ["std::string", "void"],
    ['"../h/Type.h"', '"../h/Typename.h"', "<memory>", "<vector>"]
)

generate_tree(
    "Stmt",
    {
        "Block":        ["std::vector<Stmt> statements"],
        "Break":        ["Token keyword"],
        "Continue":     ["Token keyword"],
        "Each":         ["Token name", "Expr object", "std::vector<Stmt> body"],
        "Expression":   ["Expr expr"],
        "For":          ["Stmt initializer", "Expr condition", "Expr increment", "std::vector<Stmt> body", "Token keyword"],
        "Function":     ["Token name", "std::unique_ptr<Typename> returnTypename", "std::vector<std::unique_ptr<Typename>> params", "std::vector<Stmt> body", "Type type"],
        "Given":        ["Expr value", "std::vector<GivenCase> cases"],
        "If":           ["Expr condition", "std::vector<Stmt> thenBlock", "std::vector<Stmt> elseBlock", "Token keyword"],
        "Return":       ["Token keyword", "Expr value"],
        "Struct":       ["Token name", "std::vector<Token> traits", "std::vector<std::unique_ptr<Typename>> fields", "std::vector<std::shared_ptr<FunctionStmt>> methods", "std::vector<std::shared_ptr<FunctionStmt>> assocFunctions"],
        "Trait":        ["Token name", "std::vector<std::shared_ptr<FunctionStmt>> methods"],
        "While":        ["Expr condition", "std::vector<Stmt> body", "Token keyword"],
        "Variable":     ["Token name", "Typename typeName", "Expr initializer", "bool isConst"],
    },
    ["std::string", "void"],
    ['"Expr.h"', '"../h/trivialStructs.h"']
)
