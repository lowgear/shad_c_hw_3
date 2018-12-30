#include <stdlib.h>

#include "models.h"
#include "builtins.h"
#include "evaluation.h"
#include "utils/goodies.h"

#define IMPL_HEAD(name) enum OpRetCode name##_impl(struct ArgV *argv, struct ArgNames *argNames, struct Object **out)

#define BUILTIN_DEF(name, ac) \
IMPL_HEAD(name); \
struct Function name = { \
        .funcType = BuiltIn, \
        .builtin = name##_impl, \
        .argc = ac \
}; \
IMPL_HEAD(name)

BUILTIN_DEF(_if, 3) {
    struct Object *pred;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], argNames, &pred);
    if (rc != Ok)
        return rc;
    if (pred->type != Int)
        return ArgTypeMismatch;

    return GetLazyExprVal(&argv->array[pred ? 1 : 2], argNames, out);
}

BUILTIN_DEF(cons, 2) {
    struct Object *x, *y;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], argNames, &x);
    if (rc != Ok)
        return rc;
    rc = GetLazyExprVal(&argv->array[0], argNames, &y);
    if (rc != Ok)
        return rc;
    *out = NEW(struct Object);
    if (*out == NULL)
        return AllocationFailure;
    **out =
            (struct Object) {
                    .type = Pair,
                    .pair = {
                            .first = x,
                            .second=y
                    }
            };
    return Ok;
}

BUILTIN_DEF()
