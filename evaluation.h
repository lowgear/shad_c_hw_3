#pragma once

#include <string.h>

#include "models.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        struct Object **out);

enum OpRetCode EvalCall(struct LazyExpr *lz, struct State *state);

enum OpRetCode EvalExpr(struct LazyExpr *lz, struct State *state);
