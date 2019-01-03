#pragma once

#include "models.h"

struct IdentifierValuePair
        define,

        addition,
        subtraction,
        multiplication,
        division,
        modulo,

        less,
        greater,
        equal,

        cons,
        car,
        cdr,

        _if;

extern struct IdentifierValuePair *BUILTINS[];
size_t BUILTINS_SIZE;
