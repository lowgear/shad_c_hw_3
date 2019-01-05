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
        .builtIn = n##_impl, \
        .isUserDefined = NULL, \
        .argc = ac, \
        .name = #idnt \
}; \
struct Object n##_object = { \
        .type = Func, \
        .function = &n##_func, \
        .refCnt = 2 \
}; \
struct IdentifierValuePair n = { \
        .identifier = #idnt, \
        .value = &n##_object \
}; \
IMPL_HEAD(n)

#define ALLOCRES \
struct Object *res; \
NEWSMRT(res, struct Object, return AllocationFailure);

#define PROPER_END do { \
    *out = res; \
    return Ok; \
    \
    freeRes: \
    FreeObj(&res); \
    return AllocationFailure; \
} while (0)

BUILTIN_DEF(_if, if, 3) {
    enum OpRetCode rc;
    GETARGT(pred, 0, Int)

    rc = GetLazyExprVal(argv->array[pred->integer ? 1 : 2], state, out);
    FreeObj(&pred);
    return rc;
}

BUILTIN_DEF(cons, cons, 2) {
    ALLOCRES
    struct Pair *pair;
    TRY_NEW(pair, struct Pair, goto freeRes);
    CPYREF(argv->array[0], pair->first);
    CPYREF(argv->array[1], pair->second);
    res->type = Pair;
    res->pair = pair;
    PROPER_END;
}

BUILTIN_DEF(car, car, 1) {
    enum OpRetCode rc;
    GETARGT(p, 0, Pair)
    rc = GetLazyExprVal(p->pair->first, state, out);
    FreeObj(&p);
    return rc;
}

BUILTIN_DEF(cdr, cdr, 1) {
    enum OpRetCode rc;
    GETARGT(p, 0, Pair)
    rc = GetLazyExprVal(p->pair->second, state, out);
    FreeObj(&p);
    return rc;
}

IMPL_HEAD(function) {
    return SyntaxViolation;
}

struct Function function_func = {
        .name = "function",
        .isUserDefined = NULL,
        .builtIn = function_impl,
        .argc = 0,
};
struct Object badFunc = {
        .type = Func,
        .function = &function_func,
        .refCnt = 2
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

    char *funcName = NEWARR(char, strlen(ID(header->paramsV, 0)->var) + 1);
    if (funcName == NULL)
        return AllocationFailure;
    strcpy(funcName, ID(header->paramsV, 0)->var);

    struct ArgNames *argNames;
    const uint8_t argc = (const uint8_t) (header->paramsV->size - 1);
    INIT_ARR(argNames, argc, goto allocFailure);
    for (size_t i = 1; i < header->paramsV->size; ++i) {
        ID(argNames, i - 1) = NEWARR(char, strlen(ID(header->paramsV, i)->var) + 1);
        if (ID(argNames, i - 1) == NULL) goto cleanup;
        strcpy(ID(argNames, i - 1), ID(header->paramsV, i)->var);
        continue;

        cleanup:
        for (size_t j = 1; j < i; ++j) {
            free(ID(argNames, i - 1));
        }
        goto freeArgNames;
    }

    struct Function *function;
    TRY_NEW(function, struct Function, goto freeArgNames);
    *function = (struct Function) {
            .name=funcName,
            .userDef = (struct UserDefFunc) {
                    .head = argNames
            },
            .argc = argc
    };
    CPYREF(body, function->userDef.body);

    struct Object *func;
    NEWSMRT(func, struct Object, goto freeFunction);
    func->type = Func;
    func->function = function;

    struct IdentifierValuePair iv = {
            .identifier = funcName,
            .value = func
    };
    PUSH_BACK_P(&state->identifiers, iv, goto freeFunc);

    *out = &badFunc;

    return Ok;

    freeFunc:
    free(func);

    freeFunction:
    free(function);

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
    res->type = Int; \
    res->integer = x->integer idft y->integer; \
    FreeObj(&x); \
    FreeObj(&y); \
    PROPER_END; \
}

BINOP(addition, +)

//BINOP(subtraction, -)

BUILTIN_DEF(subtraction, -, 2) {
    \
    enum OpRetCode rc; \
    GETARGT(x, 0, Int)
    GETARGT(y, 1, Int)
    ALLOCRES
    res->type = Int;
    res->integer = x->integer - y->integer;
    FreeObj(&x);
    FreeObj(&y);
    PROPER_END;
}

BINOP(multiplication, *)


BUILTIN_DEF(division, /, 2) {
    enum OpRetCode rc;
    GETARGT(y, 1, Int)
    if (y->integer == 0) {
        FreeObj(&y);
        return DBZ;
    }
    GETARGT(x, 0, Int)
    ALLOCRES
    res->type = Int;
    res->integer = x->integer / y->integer;
    FreeObj(&x);
    FreeObj(&y);
    PROPER_END;
}


BUILTIN_DEF(modulo, %, 2) {
    enum OpRetCode rc;
    GETARGT(y, 1, Int)
    if (y->integer == 0) {
        FreeObj(&y);
        return DBZ;
    }
    GETARGT(x, 0, Int)
    ALLOCRES
    res->type = Int;
    res->integer = x->integer % y->integer;
    FreeObj(&x);
    FreeObj(&y);
    PROPER_END;
}

#define CMPOP(name, idft, op) \
BUILTIN_DEF(name, idft, 2) { \
    enum OpRetCode rc; \
    GETARGT(x, 0, Int) \
    GETARGT(y, 1, Int) \
    ALLOCRES \
    res->type = Int; \
    res->integer = (x->integer op y->integer ? 1 : 0); \
    FreeObj(&x); \
    FreeObj(&y); \
    PROPER_END; \
}

CMPOP(less, <, <)

CMPOP(greater, >, >)

//CMPOP(equal, =, ==)

BUILTIN_DEF(equal, =, 2) {
    enum OpRetCode rc;
    GETARGT(x, 0, Int)
    GETARGT(y, 1, Int)
    ALLOCRES
    res->type = Int;
    res->integer = x->integer == y->integer;
    FreeObj(&x);
    FreeObj(&y);
    PROPER_END;
}

struct Object NullObject = {
        .type = Null,
        .refCnt = 2
};
struct IdentifierValuePair nul = {
        .identifier = "null",
        .value = &NullObject
};
