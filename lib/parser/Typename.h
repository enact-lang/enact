#ifndef ENACT_TYPENAME_H
#define ENACT_TYPENAME_H

#include <optional>
#include <string>

#include "Token.h"

namespace enact {
    class Typename {
    public:
        enum class Kind {
            BASIC,
            VARIABLE,
            PARAMETRIC,
            TUPLE,
            FUNCTION,
            REFERENCE,
            OPTIONAL
        };

        virtual ~Typename() = default;

        virtual std::unique_ptr<Typename> clone() const = 0;
        virtual Kind kind() const = 0;

        virtual const std::string& name() const = 0;
        virtual const Token& where() const = 0;
    };

    class BasicTypename : public Typename {
    public:
        explicit BasicTypename(Token name);
        explicit BasicTypename(std::string name, Token where);
        BasicTypename(const BasicTypename& typename_);

        ~BasicTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

    private:
        std::string m_name;
        Token m_where;
    };

    class VariableTypename : public Typename {
    public:
        explicit VariableTypename(Token identifier);
        VariableTypename(const VariableTypename& typename_);

        ~VariableTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

    private:
        Token m_identifier;

        std::string m_name;
    };

    class ParametricTypename : public Typename {
    public:
        ParametricTypename(
                std::unique_ptr<const Typename> constructorTypename,
                std::vector<std::unique_ptr<const Typename>> parameterTypenames);
        ParametricTypename(const ParametricTypename& typename_);

        ~ParametricTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& constructorTypename() const;
        const std::vector<std::unique_ptr<const Typename>>& parameterTypenames() const;

    private:
        std::unique_ptr<const Typename> m_constructorTypename;
        std::vector<std::unique_ptr<const Typename>> m_parameterTypenames;

        std::string m_name;
    };

    class TupleTypename : public Typename {
    public:
        TupleTypename(
                std::vector<std::unique_ptr<const Typename>> elementTypenames,
                Token paren);
        TupleTypename(const TupleTypename& typename_);

        ~TupleTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const std::vector<std::unique_ptr<const Typename>>& elementTypenames() const;

    private:
        std::vector<std::unique_ptr<const Typename>> m_elementTypenames;
        Token m_paren;

        std::string m_name;
    };

    class FunctionTypename : public Typename {
    public:
        FunctionTypename(std::unique_ptr<const Typename> returnTypename,
                         std::vector<std::unique_ptr<const Typename>> argumentTypenames);
        FunctionTypename(const FunctionTypename& typename_);

        ~FunctionTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& returnTypename() const;
        const std::vector<std::unique_ptr<const Typename>>& argumentTypenames() const;

    private:
        std::unique_ptr<const Typename> m_returnTypename;
        std::vector<std::unique_ptr<const Typename>> m_argumentTypenames;
        std::string m_name;
    };

    class ReferenceTypename : public Typename {
    public:
        ReferenceTypename(
                std::optional<Token> permission,
                std::optional<Token> region,
                std::unique_ptr<const Typename> referringTypename);
        ReferenceTypename(const ReferenceTypename& typename_);

        ~ReferenceTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const std::optional<Token>& permission() const;
        const std::optional<Token>& region() const;
        const std::unique_ptr<const Typename>& referringTypename() const;

    private:
        std::optional<Token> m_permission;
        std::optional<Token> m_region;
        std::unique_ptr<const Typename> m_referringTypename;
        std::string m_name;
    };

    class OptionalTypename : public Typename {
    public:
        explicit OptionalTypename(std::unique_ptr<const Typename> wrappedTypename);
        OptionalTypename(const OptionalTypename& typename_);

        ~OptionalTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& wrappedTypename() const;

    private:
        std::unique_ptr<const Typename> m_wrappedTypename;

        std::string m_name;
    };
}

#endif //ENACT_TYPENAME_H
