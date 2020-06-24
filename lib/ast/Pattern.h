#ifndef ENACT_PATTERN_H
#define ENACT_PATTERN_H

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

template<class R>
class PatternVisitor {
public:
    R visitPattern(Pattern& pattern) {
        return pattern.accept(*this);
    }

    virtual R visitValuePattern(ValuePattern& pattern) = 0;
    // TODO: add other pattern types
};


#endif //ENACT_PATTERN_H
