#pragma once

#include <string.h>

#include "models.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        const struct Object **out);

enum OpRetCode EvalCall(
        struct CallArgV *callArgV,
        struct ArgV *argv,
        const struct ArgNames *argNames,
        struct State *state,
        const struct Object **out);

enum OpRetCode EvalExpr(
        const struct Expression *expression,
        struct ArgV *argv,
        const struct ArgNames *argNames,
        struct State *state,
        const struct Object **out);
