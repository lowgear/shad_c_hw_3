#pragma once

#include "models.h"

extern struct IdentifierValuePair
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

        _if,

        nul;

extern struct IdentifierValuePair *BUILTINS[];
extern size_t BUILTINS_SIZE;
