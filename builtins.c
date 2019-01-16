#include <stdlib.h>
#include <string.h>

#include "models.h"
#include "builtins.h"
#include "evaluation.h"
#include "utils/new_tools.h"
#include "impl_goodies.h"
#include "utils/error_check_tools.h"

#define IMPL_HEAD(name) enum OpRetCode name##_impl( \
    struct LazyExpr *lz, \
    struct ArgV *argv, \
    struct State *state, \
    struct ArgV *localArgv, \
    struct ArgNames *localAN)

#define BUILTIN_DEF(n, idnt, ac) \
IMPL_HEAD(n); \
struct Function n##_func = {\
        .builtIn.func = n##_impl, \
        .builtIn.isUserDefined = NULL, \
        .argc = ac, \
        .name = #idnt \
}; \
struct Object n##_object = { \
        .type = Func, \
        .function = &n##_func, \
        .refCnt = 2 \
}; \
struct LazyExpr n##_lz = { \
    .argNames = NULL, \
    .argv = NULL, \
    .expression = NULL, \
    .value = &n##_object, \
    .refCnt = 1, \
}; \
struct IdentifierValuePair n = { \
        .identifier = #idnt, \
        .value = &n##_lz \
}; \
IMPL_HEAD(n)

#define ALLOCRES \
struct Object *res; \
NEWSMRT(res, struct Object, return AllocationFailure); // todo

#define PROPER_END do { \
    lz->value = res; \
    return Ok; \
    \
    freeRes: \
    free(res); \
    return AllocationFailure; \
} while (0)

#define PROPER_END_NOERR do { \
    lz->value = res; \
    return Ok; \
} while (0)

BUILTIN_DEF(_if, if, 3) {
    enum OpRetCode rc;
    GETARGT(pred, 0, Int)

    FreeExpr(&lz->expression);
    FREE_A(lz->argv, FreeLazyExpr);
    FREE_A(lz->argNames, SUB_FREE);

    struct LazyExpr *const r = ID(argv, pred->integer ? 1 : 2);

    CPYREF(r->expression, lz->expression);
    CPYREF(r->argv, lz->argv);
    CPYREF(r->argNames, lz->argNames);
    CPYREF(r->value, lz->value);

    FreeObj(&pred);
    return Ok;
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
    rc = GetLazyExprVal(p->pair->first, state, &lz->value);
    FreeObj(&p);
    return rc;
}

BUILTIN_DEF(cdr, cdr, 1) {
    enum OpRetCode rc;
    GETARGT(p, 0, Pair)
    rc = GetLazyExprVal(p->pair->second, state, &lz->value);
    FreeObj(&p);
    return rc;
}

IMPL_HEAD(function) {
    return SyntaxViolation;
}

struct Function function_func = {
        .name = "function",
        .builtIn.isUserDefined = NULL,
        .builtIn.func = function_impl,
        .argc = 0,
};
struct Object defineFunc = {
        .type = Func,
        .function = &function_func,
        .refCnt = 1
};

IMPL_HEAD(variable) {
    return SyntaxViolation;
}

enum OpRetCode CheckDefineHead(struct State *state, struct Expression *header) {
    if (header->expType == Var) {
        if (IsRedefinition(state, header->var))
            return IdentifierRedefinition;
        return Ok;
    }

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
    if (IsRedefinition(state, headArr[0]->var))
        return IdentifierRedefinition;
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

bool IsRedefinition(struct State *state, char *name) {
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        if (strcmp(name, ID(state->identifiers, i).identifier) == 0)
            return true;
    }
    for (size_t i = 0; i < CNT(state->builtins); ++i) {
        if (strcmp(name, ID(state->builtins, i).identifier) == 0)
            return true;
    }
    return false;
}

enum OpRetCode DefineFuncImpl(
        struct Expression *header,
        struct Expression *body,
        struct State *state,
        struct Object **out) {
    enum OpRetCode rc;
    char *funcName;
    TRY_NEWARR(funcName, char, strlen(ID(header->paramsV, 0)->var) + 1, rc = AllocationFailure;
            goto exit;);
    char *identifier;
    TRY_NEWARR(identifier, char, strlen(ID(header->paramsV, 0)->var) + 1, rc = AllocationFailure;
            goto freeFuncName;);
    strcpy(funcName, ID(header->paramsV, 0)->var);
    strcpy(identifier, ID(header->paramsV, 0)->var);

    struct ArgNames *argNames;
    const uint8_t argc = (const uint8_t) (header->paramsV->size - 1);
    INIT_ARR(argNames, argc, rc = AllocationFailure;
            goto freeIdentifier;);
    for (size_t i = 1; i < header->paramsV->size; ++i) {
        TRY_NEWARR(ID(argNames, i - 1), char, strlen(ID(header->paramsV, i)->var) + 1, goto cleanup;);
        strcpy(ID(argNames, i - 1), ID(header->paramsV, i)->var);
        continue;

        cleanup:
        rc = AllocationFailure;
        for (size_t j = 1; j < i; ++j) {
            free(ID(argNames, i - 1));
        }
        free(argNames);
        goto freeIdentifier;
    }

    struct Function *function;
    TRY_NEW(function, struct Function, rc = AllocationFailure;
            goto freeArgNames;);
    *function = (struct Function) {
            .name=funcName,
            .userDef = (struct UserDefFunc) {
                    .head = argNames
            },
            .argc = argc
    };
    CPYREF(body, function->userDef.body);

    struct Object *func;
    NEWSMRT(func, struct Object, rc = AllocationFailure;
            goto freeFunction;);
    func->type = Func;
    func->function = function;

    struct LazyExpr *lz;
    NEWSMRT(lz, struct LazyExpr, rc = AllocationFailure;
            goto freeFunc);
    lz->value = func;
    lz->expression = NULL;
    lz->argv = NULL;
    lz->argNames = NULL;

    struct IdentifierValuePair iv = {
            .identifier = identifier,
            .value = lz
    };
    PUSH_BACK_P(&state->identifiers, iv, rc = AllocationFailure;
            goto freeLz;);

    CPYREF(&defineFunc, *out);

    return Ok;

    freeLz:
    FreeLazyExpr(&lz);

    freeFunc:
    free(func);

    freeFunction:
    free(function);

    freeArgNames:
    FREE_A(argNames, SUB_FREE);

    freeIdentifier:
    free(identifier);

    freeFuncName:
    free(funcName);

    exit:
    return rc;
}

enum OpRetCode DefineVarImpl(
        struct Expression *header,
        struct Expression *body,
        struct State *state,
        struct Object **out) {
    enum OpRetCode rc;
    struct LazyExpr *lz;
    NEWSMRT(lz, struct LazyExpr, rc = AllocationFailure;
            goto exit);
    CPYREF(body, lz->expression);
    CPYREF(&emptyArgV, lz->argv);
    CPYREF(&emptyArgNames, lz->argNames);
    lz->value = NULL;

    char *identifier;
    TRY_NEWARR(identifier, char, strlen(header->var) + 1, rc = AllocationFailure;
            goto freeLz;);
    strcpy(identifier, header->var);

    struct IdentifierValuePair iv = {
            .identifier = identifier,
            .value = lz
    };

    PUSH_BACK_P(&state->identifiers, iv, rc = AllocationFailure;
            goto freeLz);

    CPYREF(&defineFunc, *out);

    return Ok;

    freeLz:
    FreeLazyExpr(&lz);

    exit:
    return rc;
}

BUILTIN_DEF(define, define, 2) {
    struct Expression *header = ID(argv, 0)->expression;
    struct Expression *body = ID(argv, 1)->expression;
    enum OpRetCode rc;
    rc = CheckDefineHead(state, header);
    if (rc != Ok)
        return rc;

    if (header->expType == Call) {
        return DefineFuncImpl(header, body, state, &lz->value);
    }
    return DefineVarImpl(header, body, state, &lz->value);
}

void ApplyLetDef(char *name, struct Expression *replacement, struct Expression **expr) {
    if ((*expr)->expType == Var && strcmp((*expr)->var, name) == 0) {
        FreeExpr(expr);
        CPYREF(replacement, *expr);
        return;
    }

    if ((*expr)->expType == Call) {
        for (size_t i = 0; i < SIZE((*expr)->paramsV); ++i) {
            ApplyLetDef(name, replacement, &ID((*expr)->paramsV, i));
        }
    }
}

BUILTIN_DEF(let, let, VARIADIC_ARGS) {
    if (SIZE(argv) < 1)
        return ArgNumberMismatch;
    const size_t defNum = SIZE(argv) - 1;
    for (size_t i = 0; i < defNum; ++i) {
        struct Expression *const cur = ID(argv, i)->expression;
        if (cur->expType != Call)
            return ArgTypeMismatch;
        if (SIZE(cur->paramsV) != 2)
            return ArgNumberMismatch;
        struct Expression *const cur_name = ID(cur->paramsV, 0);
        if (cur_name->expType != Var)
            return ArgTypeMismatch;
        if (IsRedefinition(state, cur_name->var))
            return IdentifierRedefinition;
        for (size_t j = 0; j < SIZE(localAN); ++j) {
            if (strcmp(cur_name->var, ID(localAN, j)) == 0)
                return IdentifierRedefinition;
        }
    }

    for (size_t i = 0; i < defNum; ++i) {
        for (size_t j = i + 1; j < defNum; ++j) {
            if (strcmp(ID(ID(argv, i)->expression->paramsV, 0)->var,
                       ID(ID(argv, j)->expression->paramsV, 0)->var) == 0)
                return IdentifierRedefinition;
        }
    }

    for (size_t i = 0; i < defNum; ++i) {
        struct CallParams *cur = ID(argv, i)->expression->paramsV;
        ApplyLetDef(ID(cur, 0)->var, ID(cur, 1), &ID(argv, defNum)->expression);
    }

    return EvalExpr(lz, state);
}

enum OpRetCode CheckLambdaHead(struct Expression *header) {
    if (header->expType != Call)
        return SyntaxViolation;
    const size_t headerLen = header->paramsV->size;
    if (headerLen < 1)
        return SyntaxViolation;
    if (headerLen - 1 > UINT8_MAX)
        return SyntaxViolation;

    struct Expression *const *headArr = header->paramsV->array;
    for (size_t i = 0; i < headerLen; ++i) {
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

void HandleClosures(struct ArgV *argv, struct ArgNames *argNames, struct Expression *body) {
    if (body->expType == Const)
        return;
    switch (body->expType) {
        case Call:
            for (size_t i = 0; i < SIZE(body->paramsV); ++i) {
                HandleClosures(argv, argNames, ID(body->paramsV, i));
            }
            break;
        case Const:
            return;
        case Var:
            for (size_t i = 0; i < SIZE(argNames); ++i) {
                if (strcmp(ID(argNames, i), body->var) == 0) {
                    free(body->var);
                    body->expType = Const;
                    CPYREF(ID(argv, i), body->object);
                }
            }
            break;
    }
}

BUILTIN_DEF(lambda, lambda, 2) {
    struct Expression *header = ID(argv, 0)->expression;
    struct Expression *body = ID(argv, 1)->expression;
    enum OpRetCode rc;
    rc = CheckLambdaHead(header);
    if (rc != Ok)
        return rc;

    struct ArgNames *argNames;
    const uint8_t argc = (const uint8_t) (header->paramsV->size);
    INIT_ARR(argNames, argc, rc = AllocationFailure;
            goto exit;);
    for (size_t i = 0; i < SIZE(header->paramsV); ++i) {
        TRY_NEWARR(ID(argNames, i), char, strlen(ID(header->paramsV, i)->var) + 1, goto cleanup);
        strcpy(ID(argNames, i), ID(header->paramsV, i)->var);
        continue;

        cleanup:
        rc = AllocationFailure;
        for (size_t j = 0; j < i; ++j) {
            free(ID(argNames, i));
        }
        free(argNames);
        goto exit;
    }

    struct Function *function;
    TRY_NEW(function, struct Function, rc = AllocationFailure;
            goto freeArgNames;);
    *function = (struct Function) {
            .name = NULL,
            .userDef = (struct UserDefFunc) {
                    .head = argNames
            },
            .argc = argc
    };
    CPYREF(body, function->userDef.body);

    HandleClosures(localArgv, localAN, body);

    struct Object *func;
    NEWSMRT(func, struct Object, rc = AllocationFailure;
            goto freeFunction;);
    func->type = Func;
    func->function = function;

    lz->value = func;

    return Ok;

    freeFunction:
    free(function);

    freeArgNames:
    FREE_A(argNames, SUB_FREE);

    exit:
    return rc;
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
    PROPER_END_NOERR; \
}

BINOP(addition, +)

BINOP(subtraction, -)

BINOP(multiplication, *)


#define DIVOP(name, idft) \
BUILTIN_DEF(name, idft, 2) { \
    enum OpRetCode rc; \
    GETARGT(y, 1, Int) \
    if (y->integer == 0) { \
        FreeObj(&y); \
        return DBZ; \
    } \
    GETARGT(x, 0, Int) \
    ALLOCRES \
    res->type = Int; \
    res->integer = x->integer idft y->integer; \
    FreeObj(&x); \
    FreeObj(&y); \
    PROPER_END_NOERR; \
}

DIVOP(division, /)

DIVOP(modulo, %)

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
    PROPER_END_NOERR; \
}

CMPOP(less, <, <)

CMPOP(greater, >, >)

CMPOP(equal, =, ==)

struct Object NullObject = {
        .type = Null,
        .refCnt = 2
};
struct LazyExpr NullLz = {
        .value = &NullObject,
        .refCnt = 2
};
struct IdentifierValuePair nul = {
        .identifier = "null",
        .value = &NullLz
};

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
        &nul,
        &let,
        &lambda};

size_t BUILTINS_SIZE = sizeof(BUILTINS) / sizeof(BUILTINS[0]);
