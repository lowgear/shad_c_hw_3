#pragma once

#include <string.h>

#include "models.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct ArgNames *argNames,
        struct Object **out);

enum OpRetCode EvalCall(
        struct CallArgV *callArgV,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct Object **out);

enum OpRetCode EvalExpr(
        struct Expression *expression,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct Object **out);
