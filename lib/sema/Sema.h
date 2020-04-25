#ifndef ENACT_SEMA_H
#define ENACT_SEMA_H

namespace enact {
    // Forward declared from "../context/Context.h"
    class Context;

    class Sema {
    private:
        Context& m_context;

        // All global declarations. Assembled at parse time. We use reference
        // wrappers here relying on the assumption that the AST will be preserved
        // throughout the compilation process.
        std::vector<std::reference_wrapper<const Stmt>> m_decls{};

        // All global variables and their corresponding types and semantic info.
        // Populated (declared) by SemaDecls and resolved (defined) by SemaDefs.
        InsertionOrderMap<std::string, VariableInfo> m_variables{};

        // All global types and their resolved values. Populated (declared) by
        // SemaDecls and resolved (defined) by SemaDefs.
        InsertionOrderMap<std::string, Type> m_types{};

        // The two AST passes that we manage here.
        SemaDecls m_declarer{*this};
        SemaDefs m_definer{*this};

    public:
        explicit Sema(Context& context);

        // Parser uses this to add global declarations when it finds them.
        void addDeclStmt(const Stmt& stmt);

        // SemaDecl uses these to declare global names.
        void declareVariable(const std::string& name, const VariableInfo& info);
        void declareType(const std::string& name, Type value = nullptr);

        // SemaDef uses these to define global names.
        void defineVariable(const std::string& name, Type type);
        void defineType(const std::string& name, Type value);

        // Check if a symbol has been declared. Returns nullopt if the symbol
        // does not exist, otherwise returns the symbol in question.
        std::optional<VariableInfo> variableDeclared(const std::string& name);
        std::optional<Type> typeDeclared(const std::string& name);
    };
}

#endif //ENACT_SEMA_H
