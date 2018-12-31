#include <stdlib.h>
#include <string.h>

#include "models.h"
#include "builtins.h"
#include "evaluation.h"
#include "utils/goodies.h"

#define IMPL_HEAD(name) enum OpRetCode name##_impl(struct ArgV *argv, struct State *state, const struct Object **out)

#define BUILTIN_DEF(name, idnt, ac) \
IMPL_HEAD(name); \
struct Object name##_object = { \
        .type = Func, \
        .func = (struct Function) {\
            .funcType = BuiltIn, \
            .builtin = name##_impl, \
            .argc = ac \
        } \
}; \
struct IdentifierValuePair name = { \
        .identifier = #idnt, \
        .value = &name##_object \
}; \
IMPL_HEAD(name)

BUILTIN_DEF(_if, if, 3) {
    const struct Object *pred;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], state, &pred);
    if (rc != Ok)
        return rc;
    if (pred->type != Int)
        return ArgTypeMismatch;

    return GetLazyExprVal(&argv->array[pred ? 1 : 2], state, out);
}

BUILTIN_DEF(cons, cons, 2) {
    const struct Object *x, *y;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], state, &x);
    if (rc != Ok)
        return rc;
    rc = GetLazyExprVal(&argv->array[0], state, &y);
    if (rc != Ok)
        return rc;
    struct Object *res = NEW(struct Object);
    if (res == NULL)
        return AllocationFailure;
    *res = (struct Object) {
            .type = Pair,
            .pair = {
                    .first = x,
                    .second = y
            }
    };
    *out = res;
    return Ok;
}

BUILTIN_DEF(car, car, 1) {
    const struct Object *p;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], state, &p);
    if (rc != Ok)
        return rc;
    if (p->type != Pair)
        return ArgTypeMismatch;
    *out = p->pair.first;
    return Ok;
    // TODO free
}

BUILTIN_DEF(cdr, cdr, 1) {
    const struct Object *p;
    enum OpRetCode rc = GetLazyExprVal(&argv->array[0], state, &p);
    if (rc != Ok)
        return rc;
    if (p->type != Pair)
        return ArgTypeMismatch;
    *out = p->pair.second;
    return Ok;
    // TODO free
}

BUILTIN_DEF(define, define, 2) {
    const struct Expression *header = argv->array[0].expression;
    const struct Expression *body = argv->array[1].expression;
    if (header->expType != Call)
        return SyntaxViolation;
    const size_t headerLen = header->paramsV->size;
    if (headerLen < 1)
        return SyntaxViolation;
    if (headerLen - 1 > UINT8_MAX)
        return SyntaxViolation;

    struct Expression *const *headArr = header->paramsV->array;
    if (headArr[0]->expType != Var)
        return SyntaxViolation;
    for (size_t i = 1; i < headerLen; ++i) {
        if (headArr[i]->expType != Var)
            return ArgTypeMismatch;
        for (size_t j = i + 1; j < headerLen; ++j) {
            if (strcmp(headArr[i]->var,
                       headArr[j]->var) == 0)
                return IdentifierRedefinition;
        }
    }

    const char *funcName = headArr[0]->var;
    struct Object *func = NEW(struct Object);
    if (func == NULL)
        goto allocFailure;
    *func = (struct Object) {
            .type = Func,
            .func = (struct Function) {
                    .argc = (uint8_t) (headerLen - 1),
                    .funcType = UserDefined,
                    .expression = body
            }
    };

    struct IdentifierValuePair iv = {
            .identifier = funcName,
            .value = func
    };
    PUSH_BACK_P(&state->identifiers, iv, goto freeFunc);

    return Ok;

    freeFunc:
    free(func);
    allocFailure:
    return AllocationFailure;
}
