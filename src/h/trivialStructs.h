#ifndef ENACT_GIVENCASE_H
#define ENACT_GIVENCASE_H

#include "../ast/Stmt.h"

class StmtBase;
typedef std::shared_ptr<StmtBase> Stmt;

struct GivenCase {
    Expr value;
    std::vector<Stmt> body;
    Token keyword;
};

// A name (token) associated with a typename (string).
struct NamedTypename {
    Token name;
    std::string typeName;
};

// A name (token) associated with a type.
struct NamedType {
    Token name;
    Type type;
};


#endif //ENACT_GIVENCASE_H
