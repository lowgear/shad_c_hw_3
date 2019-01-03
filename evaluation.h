#pragma once

#include <string.h>

#include "models.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        struct Object **out);

enum OpRetCode EvalCall(struct CallParams *callParams, struct ArgV *argv, struct State *state, struct Object **out);

enum OpRetCode EvalExpr(
        struct Expression *expression,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct Object **out);
