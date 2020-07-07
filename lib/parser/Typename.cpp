#include <sstream>

#include "Typename.h"

namespace enact {
    BasicTypename::BasicTypename(Token name) :
            m_name{name.lexeme},
            m_where{std::move(name)} {}

    BasicTypename::BasicTypename(std::string name, Token where) :
            m_name{std::move(name)},
            m_where{std::move(where)} {}

    BasicTypename::BasicTypename(const BasicTypename& typeName) :
            BasicTypename{typeName.m_name, typeName.m_where} {}

    std::unique_ptr<Typename> BasicTypename::clone() const {
        return std::make_unique<BasicTypename>(*this);
    }

    Typename::Kind BasicTypename::kind() const {
        return Kind::BASIC;
    }

    const std::string& BasicTypename::name() const {
        return m_name;
    }

    const Token& BasicTypename::where() const {
        return m_where;
    }

    ParametricTypename::ParametricTypename(
            std::unique_ptr<const Typename> constructorTypename,
            std::vector<std::unique_ptr<const Typename> > parameterTypenames) :
            m_constructorTypename{std::move(constructorTypename)},
            m_parameterTypenames{std::move(parameterTypenames)},
            m_name{} {
        std::ostringstream name;
        std::string separator;

        name << m_constructorTypename->name() << '[';
        for (const auto& parameterTypename : m_parameterTypenames) {
            name << separator << parameterTypename->name();
            separator = ", ";
        }
        name << ']';

        m_name = name.str();
    }

    ParametricTypename::ParametricTypename(const ParametricTypename &typename_) :
            ParametricTypename{
                    typename_.m_constructorTypename->clone(),
                    cloneAll(typename_.m_parameterTypenames)} {
    }

    std::unique_ptr<Typename> ParametricTypename::clone() const {
        return std::make_unique<ParametricTypename>(*this);
    }

    Typename::Kind ParametricTypename::kind() const {
        return Kind::PARAMETRIC;
    }

    const std::string& ParametricTypename::name() const {
        return m_name;
    }

    const Token& ParametricTypename::where() const {
        return m_constructorTypename->where();
    }

    const Typename& ParametricTypename::constructorTypename() const {
        return *m_constructorTypename;
    }

    const std::vector<std::unique_ptr<const Typename>>& ParametricTypename::parameterTypenames() const {
        return m_parameterTypenames;
    }

    // TODO: TUPLE

    FunctionTypename::FunctionTypename(std::unique_ptr<const Typename> returnTypename,
                                       std::vector<std::unique_ptr<const Typename>> argumentTypenames) :
            m_returnTypename{std::move(returnTypename)},
            m_argumentTypenames{std::move(argumentTypenames)} {
        std::ostringstream name{};

        const bool wrap = argumentTypenames.size() != 1;

        if (wrap) name << '(';

        std::string separator{};
        for (const auto& typename_ : m_argumentTypenames) {
            name << separator;
            name << typename_->name();
            separator = ", ";
        }

        if (wrap) name << ')';
        name << " => " << m_returnTypename->name();

        m_name = name.str();
    }

    FunctionTypename::FunctionTypename(const FunctionTypename& typename_) :
            FunctionTypename{
                    typename_.m_returnTypename->clone(),
                    cloneAll(typename_.m_argumentTypenames)} {
    }

    std::unique_ptr<Typename> FunctionTypename::clone() const {
        return std::make_unique<FunctionTypename>(*this);
    }

    const Typename& FunctionTypename::returnTypename() const {
        return *m_returnTypename;
    }

    const std::vector<std::unique_ptr<const Typename>>& FunctionTypename::argumentTypenames() const {
        return m_argumentTypenames;
    }

    Typename::Kind FunctionTypename::kind() const {
        return Kind::FUNCTION;
    }

    const std::string& FunctionTypename::name() const {
        return m_name;
    }

    const Token& FunctionTypename::where() const {
        return m_returnTypename->where();
    }

    ReferenceTypename::ReferenceTypename(Token permission, Token region,
                                         std::unique_ptr<const Typename> referringTypename) :
            m_permission{std::move(permission)},
            m_region{std::move(region)},
            m_referringTypename{std::move(referringTypename)},
            m_name{"&" + m_permission.lexeme + " " + m_region.lexeme} {

    }

    ReferenceTypename::ReferenceTypename(const ReferenceTypename& typename_) :
            ReferenceTypename{
                    typename_.m_permission,
                    typename_.m_region,
                    typename_.m_referringTypename->clone()} {
    }

    std::unique_ptr<Typename> ReferenceTypename::clone() const {
        return std::make_unique<ReferenceTypename>(*this);
    }

    Typename::Kind ReferenceTypename::kind() const {
        return Kind::REFERENCE;
    }

    const std::string& ReferenceTypename::name() const {
        return m_name;
    }

    const Token& ReferenceTypename::where() const {
        return m_permission;
    }

    const Token& ReferenceTypename::permission() const {
        return m_permission;
    }

    const Token& ReferenceTypename::region() const {
        return m_region;
    }

    const std::unique_ptr<const Typename>& ReferenceTypename::referringTypename() const {
        return m_referringTypename;
    }

    // TODO: OPTIONAL
}
