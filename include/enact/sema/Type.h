#ifndef ENACT_TYPE_H
#define ENACT_TYPE_H

#include <enact/sema/Item.h>

namespace enact {
    class Type {
    public:
        enum class Kind {
            PRIMITIVE, // e.g. int, bool, u16, f32

            // (T, V) => K
            // FUNCTION,

            // struct T {}
            // STRUCT,

            // enum T {}
            // ENUM,

            // trait T {}
            TRAIT
        };

        virtual ~Type() = 0;

        // Are the types exactly the same? Could be:
        // - literally the same type ('int', 'int')
        // - the same type, just aliased ('int32', 'int')
        virtual bool isEquivalent(TypeHandle type) const = 0;

        // Are the types able to be implicitly converted to each other? Could be:
        // - converting a struct to a trait (but not the other way around)
        // - converting a narrow numeric type to a wide numeric type (e.g. int8 => int32)
        virtual bool isCompatible(TypeHandle type) const = 0;

        virtual Kind getKind() const = 0;
        virtual const Typename& getTypename() const = 0;

        //virtual const InsertionOrderMap<std::string, Item>& getItems() const = 0;
        //virtual const InsertionOrderMap<std::string, TypeHandleOf<TraitType>>& getTraits() const = 0;
    };

    class PrimitiveType : public Type {
    public:
        // Determines what the semantics of this type are
        // E.g. size, compatibility, equivalence, etc.
        enum class Kind {
            INT,
            I8,
            I16,
            I32,
            I64,

            UINT,
            U8,
            U16,
            U32,
            U64,

            FLOAT,
            F32,
            F64,

            BOOL
        };

        PrimitiveType(
                Kind kind,
                std::unique_ptr<const Typename> typename_);

        ~PrimitiveType() override = default;

        Kind getPrimitiveKind() const { return m_kind; }

        bool isEquivalent(TypeHandle type) const override;
        bool isCompatible(TypeHandle type) const override;

        Type::Kind getKind() const override          { return Type::Kind::PRIMITIVE; }
        const Typename& getTypename() const override { return *m_typename; }

        //const InsertionOrderMap<std::string, Item>& getItems() const override { return m_items; }
        //const InsertionOrderMap<std::string, TypeHandleOf<TraitType>>& getTraits() const override { return m_traits; }

    private:
        Kind m_kind;
        std::unique_ptr<const Typename> m_typename;
    };

    /*class FunctionType : public Type {
    public:
        FunctionType(
                std::shared_ptr<const Type> m_returnType,
                std::vector<std::shared_ptr<const Type>> m_argumentTypes)
    };*/
}

#endif //ENACT_TYPE_H
