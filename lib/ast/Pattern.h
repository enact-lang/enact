#ifndef ENACT_PATTERN_H
#define ENACT_PATTERN_H

namespace enact {
    // From "Expr.h":
    class Expr;

    template<class R>
    class PatternVisitor;

    class Pattern {
    public:
        virtual ~Pattern() = default;

        // We need to overload for every possible visitor return type here, as we cannot
        // have a templated virtual member function.
        virtual std::string accept(PatternVisitor<std::string> *visitor) = 0;
        virtual void accept(PatternVisitor<void> *visitor) = 0;
    };

    class ValuePattern;
    class WildcardPattern;

    template<class R>
    class PatternVisitor {
    public:
        R visitPattern(Pattern &pattern) {
            return pattern.accept(*this);
        }

        virtual R visitValuePattern(ValuePattern& pattern) = 0;
        virtual R visitWildcardPattern(WildcardPattern& pattern) = 0;
        // TODO: add other pattern types
    };

    class ValuePattern : public Pattern {
    public:
        std::unique_ptr<Expr> value;

        ValuePattern(std::unique_ptr<Expr> value) :
                value{std::move(value)} {}

        ~ValuePattern() override = default;

        std::string accept(PatternVisitor<std::string>* visitor) override {
            return visitor->visitValuePattern(*this);
        }

        void accept(PatternVisitor<void>* visitor) override {
            return visitor->visitValuePattern(*this);
        }
    };

    class WildcardPattern : public Pattern {
    public:
        Token keyword;

        WildcardPattern(Token keyword) :
                keyword{std::move(keyword)} {}

        ~WildcardPattern() override = default;

        std::string accept(PatternVisitor<std::string>* visitor) override {
            return visitor->visitWildcardPattern(*this);
        }

        void accept(PatternVisitor<void>* visitor) override {
            return visitor->visitWildcardPattern(*this);
        }
    };
}

#endif //ENACT_PATTERN_H
