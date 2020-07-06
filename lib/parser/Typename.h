#ifndef ENACT_TYPENAME_H
#define ENACT_TYPENAME_H

#include <string>

#include "Token.h"

namespace enact {
    class Typename {
    public:
        enum class Kind {
            BASIC,
            ARRAY,
            REFERENCE,
            FUNCTION,
            CONSTRUCTOR
        };

        virtual ~Typename() = default;

        virtual std::unique_ptr<Typename> clone() const = 0;
        virtual Kind kind() const = 0;

        virtual const std::string& name() const = 0;
        virtual const Token& where() const = 0;
    };

    class BasicTypename : public Typename {
        std::string m_name;
        Token m_where;

    public:
        explicit BasicTypename(Token name);
        explicit BasicTypename(std::string name, Token where);
        BasicTypename(const BasicTypename& typeName);

        ~BasicTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;
    };

    class ArrayTypename : public Typename {
        std::unique_ptr<const Typename> m_elementTypename;
        std::string m_name;

    public:
        explicit ArrayTypename(std::unique_ptr<const Typename> elementTypename);
        ArrayTypename(const ArrayTypename& typeName);

        ~ArrayTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& elementTypename() const;
    };

    class ReferenceTypename : public Typename {
        Token m_permission;
        Token m_region;
        std::unique_ptr<const Typename> m_referringTypename;
        std::string m_name;

    public:
        ReferenceTypename(Token permission, Token region, std::unique_ptr<const Typename> referringTypename);
        ReferenceTypename(const ReferenceTypename& typename_);

        ~ReferenceTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Token& permission() const;
        const Token& region() const;
        const std::unique_ptr<const Typename>& referringTypename() const;
    };

    class FunctionTypename : public Typename {
        std::unique_ptr<const Typename> m_returnTypename;
        std::vector<std::unique_ptr<const Typename>> m_argumentTypenames;
        std::string m_name;

    public:
        FunctionTypename(std::unique_ptr<const Typename> returnTypename,
                         std::vector<std::unique_ptr<const Typename>> argumentTypenames);
        FunctionTypename(const FunctionTypename& typeName);

        ~FunctionTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& returnTypename() const;
        const std::vector<std::unique_ptr<const Typename>>& argumentTypenames() const;
    };

    class ConstructorTypename : public Typename {
        std::unique_ptr<const Typename> m_structTypename;
        std::string m_name;

    public:
        explicit ConstructorTypename(std::unique_ptr<const Typename> structTypename);
        ConstructorTypename(const ConstructorTypename& typeName);

        ~ConstructorTypename() override = default;

        std::unique_ptr<Typename> clone() const override;
        Kind kind() const override;

        const std::string& name() const override;
        const Token& where() const override;

        const Typename& structTypename() const;
    };
}

#endif //ENACT_TYPENAME_H
