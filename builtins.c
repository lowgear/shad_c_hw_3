#include <stdlib.h>
#include <string.h>

#include "models.h"
#include "builtins.h"
#include "evaluation.h"
#include "utils/goodies.h"
#include "impl_goodies.h"

struct IdentifierValuePair *BUILTINS[] = {
        &define,
        &addition,
        &subtraction,
        &multiplication,
        &division,
        &modulo,
        &less,
        &greater,
        &equal,
        &cons,
        &car,
        &cdr,
        &_if,
        &nul};

size_t BUILTINS_SIZE = sizeof(BUILTINS) / sizeof(BUILTINS[0]);

#define IMPL_HEAD(name) enum OpRetCode name##_impl(struct ArgV *argv, struct State *state, struct Object **out)

#define BUILTIN_DEF(n, idnt, ac) \
IMPL_HEAD(n); \
struct Function n##_func = {\
        .type = BuiltIn, \
        .builtIn = n##_impl, \
        .argc = ac, \
        .name = #idnt \
}; \
struct Object n##_object = { \
        .type = Func, \
        .function = &n##_func \
}; \
struct IdentifierValuePair n = { \
        .identifier = #idnt, \
        .value = &n##_object \
}; \
IMPL_HEAD(n)

#define ALLOCRES \
struct Object *res = NEW(struct Object); \
if (res == NULL) return AllocationFailure;

#define PROPER_END do { \
    PUSH_BACK_P(&(state->objects), res, goto freeRes); \
    *out = res; \
    return Ok; \
    \
    freeRes: \
    free(res); \
    return AllocationFailure; \
} while (0)

BUILTIN_DEF(_if, if, 3) {
    enum OpRetCode rc;
    GETARGT(pred, 0, Int)

    return GetLazyExprVal(argv->array[pred->integer ? 1 : 2], state, out);
}

BUILTIN_DEF(cons, cons, 2) {
    ALLOCRES
    *res = (struct Object) {
            .type = Pair,
            .pair = {
                    .first = argv->array[0],
                    .second = argv->array[1]
            }
    };
    PROPER_END;
}

BUILTIN_DEF(car, car, 1) {
    enum OpRetCode rc;
    GETARGT(p, 0, Pair)
    return GetLazyExprVal(p->pair.first, state, out);
}

BUILTIN_DEF(cdr, cdr, 1) {
    enum OpRetCode rc;
    GETARGT(p, 0, Pair)
    return GetLazyExprVal(p->pair.second, state, out);
}

IMPL_HEAD(function) {
    return SyntaxViolation;
}

struct Function function_func = {
        .name = "function",
        .type = BuiltIn,
        .builtIn = function_impl,
        .argc = 0
};
struct Object badFunc = {
        .type = Func,
        .function = &function_func
};

enum OpRetCode CheckHead(struct Expression *header) {
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
    return Ok;
}

BUILTIN_DEF(define, define, 2) {
    struct Expression *header = ID(argv, 0)->expression;
    struct Expression *body = ID(argv, 1)->expression;
    enum OpRetCode rc;
    rc = CheckHead(header);
    if (rc != Ok)
        return rc;

    char *funcName = ID(header->paramsV, 0)->var;

    struct ArgNames *argNames;
    const uint8_t argc = (const uint8_t) (header->paramsV->size - 1);
    INIT_ARR(argNames, argc, goto allocFailure);
    for (size_t i = 1; i < header->paramsV->size; ++i) {
        ID(argNames, i - 1) = ID(header->paramsV, i)->var;
    }

    // prevent body free
    struct Expression *trueBody;
    TRY_NEW(trueBody, struct Expression, goto freeArgNames);
    *trueBody = *body;

    struct Function *function;
    TRY_NEW(function, struct Function, goto freeTrueBody);
    *function = (struct Function) {
            .name=funcName,
            .type = UserDef,
            .userDef = (struct UserDefFunc) {
                    .head = argNames,
                    .body = trueBody
            },
            .argc = argc
    };

    struct Object *func;
    TRY_NEW(func, struct Function, goto freeFunction);
    *func = (struct Object) {
            .type = Func,
            .function = function
    };

    struct IdentifierValuePair iv = {
            .identifier = funcName,
            .value = func
    };
    PUSH_BACK_P(&state->identifiers, iv, goto freeFunc);

    // finish body free prevent
    body->expType = Const;
    body->object = NULL;

    // prevent name and argNames freeing
    for (size_t i = 0; i < SIZE(header->paramsV); ++i) {
        ID(header->paramsV, i)->object = NULL;
        ID(header->paramsV, i)->expType = Const;
    }

    *out = &badFunc;

    return Ok;

    freeFunc:
    free(func);

    freeFunction:
    free(function);

    freeTrueBody:
    free(trueBody);

    freeArgNames:
    free(argNames);

    allocFailure:
    return AllocationFailure;
}

#define BINOP(name, idft) \
BUILTIN_DEF(name, idft, 2) { \
    enum OpRetCode rc; \
    GETARGT(x, 0, Int) \
    GETARGT(y, 1, Int) \
    ALLOCRES \
    *res = (struct Object) { \
            .type = Int, \
            .integer = x->integer idft y->integer \
    }; \
    PROPER_END; \
}

BINOP(addition, +)

BINOP(subtraction, -)

BINOP(multiplication, *)


BUILTIN_DEF(division, /, 2) {
    enum OpRetCode rc;
    GETARGT(y, 1, Int)
    if (y->integer == 0) return DBZ;
    GETARGT(x, 0, Int)
    ALLOCRES
    *res = (struct Object) {
            .type = Int,
            .integer = x->integer / y->integer
    };
    PROPER_END;
}


BUILTIN_DEF(modulo, %, 2) {
    enum OpRetCode rc;
    GETARGT(y, 1, Int)
    if (y->integer == 0) return DBZ;
    GETARGT(x, 0, Int)
    ALLOCRES
    *res = (struct Object) {
            .type = Int,
            .integer = x->integer % y->integer
    };
    PROPER_END;
}

#define CMPOP(name, idft, op) \
BUILTIN_DEF(name, idft, 2) { \
    enum OpRetCode rc; \
    GETARGT(x, 0, Int) \
    GETARGT(y, 1, Int) \
    ALLOCRES \
    *res = (struct Object) { \
            .type = Int, \
            .integer = (x->integer op y->integer ? 1 : 0) \
    }; \
    PROPER_END; \
}

CMPOP(less, <, <)

CMPOP(greater, >, >)

CMPOP(equal, =, ==)

struct Object NullObject = {
        .type = Null
};
struct IdentifierValuePair nul = {
        .identifier = "null",
        .value = &NullObject
};
