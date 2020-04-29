#include "Sema.h"

namespace enact {
    Sema::Sema(Context& context) : m_context{context} {
    }

    void Sema::addDeclStmt(const Stmt& stmt) {
        m_decls.emplace_back(stmt);
    }

    void Sema::declareVariable(const std::string& name, const VariableInfo& info) {
        m_variables.emplaceOrAssign(name, info);
    }

    void Sema::declareType(const std::string& name, Type value) {
        m_types.emplaceOrAssign(name, value);
    }

    void Sema::defineVariable(const std::string &name, Type type) {
        VariableInfo& info = m_variables[name];
        info.type = type;
        info.isInitialised = true;
    }

    void Sema::defineType(const std::string& name, Type value) {
        m_types[name] = type;
    }

    std::optional<VariableInfo> Sema::variableDeclared(const std::string& name) {
        return m_variables.at(name);
    }

    std::optional<Type> Sema::typeDeclared(const std::string& name) {
        return m_types.at(name);
    }
}