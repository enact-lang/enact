#ifndef ENACT_GIVENCASE_H
#define ENACT_GIVENCASE_H

#include "parser/Typename.h"

namespace enact {
    class Expr;
    class Stmt;

    struct GivenCase {
        std::unique_ptr<Expr> value;
        std::vector<std::unique_ptr<Stmt>> body;
        Token keyword;
    };

    struct Param {
        Token name;
        std::unique_ptr<const Typename> typeName;
    };

    struct Field {
        Token name;
        std::unique_ptr<const Typename> typeName;
    };
}

#endif //ENACT_GIVENCASE_H
