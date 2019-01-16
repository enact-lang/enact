#ifndef ENACT_GIVENCASE_H
#define ENACT_GIVENCASE_H

#include "Stmt.h"

class Stmt;

struct GivenCase {
    std::shared_ptr<Expr> value;
    std::vector<std::shared_ptr<Stmt>> body;
};

struct Parameter {
    Token name;
    std::string typeName;
};

#endif //ENACT_GIVENCASE_H
