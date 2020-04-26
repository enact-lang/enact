#ifndef ENACT_SEMA_H
#define ENACT_SEMA_H

namespace enact {
    // Forward declared from "../context/Context.h"
    class Context;

    class Sema {
    private:
        Context& m_context;

        // All global variables and their corresponding types and semantic info.
        // Populated (declared) by SemaDecls and resolved (defined) by SemaDefs.
        InsertionOrderMap<std::string, VariableInfo> m_globalVariables{};

        // All global types and their resolved values. Populated (declared) by
        // SemaDecls and resolved (defined) by SemaDefs.
        InsertionOrderMap<std::string, Type> m_globalTypes{};

        // The two AST passes that we manage here.
        SemaDecls m_declarer{*this};
        SemaDefs m_definer{*this};

    public:
        explicit Sema(Context& context);

        // Conduct a complete semantic analysis of `ast`, then move it back to the caller.
        std::vector<std::unique_ptr<Decl>> analyse(std::vector<std::unique_ptr<Decl> ast);

        // SemaDecl uses these to declare global names.
        void declareGlobalVariable(const std::string& name, const VariableInfo& info);
        void declareGlobalType(const std::string& name, Type value = nullptr);

        // SemaDef uses these to define global names.
        void defineGlobalVariable(const std::string& name, Type type);
        void defineGlobalType(const std::string& name, Type value);

        // Check if a symbol has been declared. Returns nullopt if the symbol
        // does not exist, otherwise returns the symbol in question.
        std::optional<VariableInfo> globalVariableDeclared(const std::string& name);
        std::optional<Type> globalTypeDeclared(const std::string& name);
    };
}

#endif //ENACT_SEMA_H
