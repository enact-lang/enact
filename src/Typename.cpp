#include <sstream>
#include "h/Typename.h"

BasicTypename::BasicTypename(Token name) :
        m_name{name.lexeme},
        m_where{std::move(name)} {}

BasicTypename::BasicTypename(std::string name, Token where) :
        m_name{std::move(name)},
        m_where{std::move(where)} {}

BasicTypename::BasicTypename(const BasicTypename& typeName) :
        BasicTypename{typeName.m_name, typeName.m_where} {}

std::unique_ptr<Typename> BasicTypename::clone() const {
    return std::make_unique<BasicTypename>(m_name, m_where);
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

ArrayTypename::ArrayTypename(std::unique_ptr<const Typename> elementTypename) :
        m_elementTypename{std::move(elementTypename)},
        m_name{m_elementTypename->name() + "[]"} {}

ArrayTypename::ArrayTypename(const ArrayTypename &typeName) :
        ArrayTypename{typeName.m_elementTypename->clone()} {}

std::unique_ptr<Typename> ArrayTypename::clone() const {
    return std::make_unique<ArrayTypename>(m_elementTypename->clone());
}

const Typename& ArrayTypename::elementTypename() const {
    return *m_elementTypename;
}

Typename::Kind ArrayTypename::kind() const {
    return Kind::ARRAY;
}

const std::string& ArrayTypename::name() const {
    return m_name;
}

const Token& ArrayTypename::where() const {
    return m_elementTypename->where();
}


FunctionTypename::FunctionTypename(std::unique_ptr<const Typename> returnTypename,
                                   std::vector<std::unique_ptr<const Typename>> argumentTypenames) :
        m_returnTypename{std::move(returnTypename)},
        m_argumentTypenames{std::move(argumentTypenames)} {
    std::stringstream name{};
    name << "fun (";

    std::string separator{};
    for (auto const& typeName : m_argumentTypenames) {
        name << separator;
        name << typeName->name();
        separator = ", ";
    }

    name << ") " << m_returnTypename->name();

    m_name = name.str();
}

FunctionTypename::FunctionTypename(const FunctionTypename &typeName) :
        m_returnTypename{typeName.m_returnTypename->clone()},
        m_argumentTypenames{} {
    for (auto& argTypeName : m_argumentTypenames) {
        m_argumentTypenames.push_back(argTypeName->clone());
    }
}

std::unique_ptr<Typename> FunctionTypename::clone() const {
    std::vector<std::unique_ptr<const Typename>> argumentTypenames{};
    for (auto& typeName : m_argumentTypenames) {
        argumentTypenames.push_back(typeName->clone());
    }

    return std::make_unique<FunctionTypename>(m_returnTypename->clone(), std::move(argumentTypenames));
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

ConstructorTypename::ConstructorTypename(std::unique_ptr<const Typename> structTypename) :
        m_structTypename{std::move(structTypename)},
        m_name{} {
    m_name = "struct " + m_structTypename->name();
}

ConstructorTypename::ConstructorTypename(const ConstructorTypename& typeName) :
        ConstructorTypename{typeName.m_structTypename->clone()} {
}

std::unique_ptr<Typename> ConstructorTypename::clone() const {
    return std::make_unique<ConstructorTypename>(m_structTypename->clone());
}

const Typename& ConstructorTypename::structTypename() const {
    return *m_structTypename;
}

Typename::Kind ConstructorTypename::kind() const {
    return Kind::CONSTRUCTOR;
}

const std::string& ConstructorTypename::name() const {
    return m_name;
}

const Token& ConstructorTypename::where() const {
    return m_structTypename->where();
}
