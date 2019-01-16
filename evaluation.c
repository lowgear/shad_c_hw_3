#include "evaluation.h"
#include "utils/new_tools.h"
#include "utils/error_check_tools.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        struct Object **out) {
    while (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr, state);
        if (rc != Ok)
            return rc;
    }
    if (out != NULL)
        CPYREF(lazyExpr->value, *out);
    return Ok;
}

enum OpRetCode MakeCallArgV(
        struct CallParams *callParams,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct ArgV **callArgV) {
    enum OpRetCode rc;

    INIT_ARR(*callArgV, callParams->size - 1, return AllocationFailure);
    for (size_t i = 1; i < callParams->size; ++i) {
        NEWSMRT(ID(*callArgV, i - 1), struct LazyExpr, rc = AllocationFailure;
                goto cleanup);
        struct LazyExpr *cur = ID(*callArgV, i - 1);
        CPYREF(ID(callParams, i), cur->expression);
        CPYREF(argv, cur->argv);
        cur->argNames = argNames;
        cur->value = NULL;

        continue;

        cleanup:
        for (size_t j = 1; j < i; ++j) {
            FreeLazyExpr(&(*callArgV)->array[i - 1]);
        }
        free(*callArgV);
        return rc;
    }
    return Ok;
}

enum OpRetCode EvalCall(struct LazyExpr *lz, struct State *state) {
    struct CallParams *const callParams = lz->expression->paramsV;
    struct ArgV *const argv = lz->argv;
    struct ArgNames *const argNames = lz->argNames;
    if (callParams->size < 1)
        return SyntaxViolation;

    enum OpRetCode rc;
    struct LazyExpr funcLz = {
            .expression = ID(callParams, 0),
            .argv = argv,
            .argNames = argNames,
            .refCnt = 1
    };
    struct Object *func;
    rc = GetLazyExprVal(&funcLz, state, &func);
    if (rc != Ok)
        return rc;
    FreeObj(&funcLz.value);
    funcLz = (struct LazyExpr) {};

    CHK(func->type == Func, rc = ArgTypeMismatch, goto freeFunc)

    CHK(callParams->size - 1 == func->function->argc || func->function->argc == VARIADIC_ARGS,
        rc = ArgNumberMismatch,
        goto freeFunc)

    struct ArgV *callArgV;
    rc = MakeCallArgV(callParams, argv, argNames, &callArgV);
    CHK(rc == Ok, (void) 0, goto freeFunc);

    if (func->function->builtIn.isUserDefined) {
        FreeExpr(&lz->expression);
        FREE_A(lz->argv, FreeLazyExpr);
        FREE_A(lz->argNames, SUB_FREE);

        CPYREF(func->function->userDef.body, lz->expression);
        CPYREF(func->function->userDef.head, lz->argNames);
        lz->argv = callArgV;

        FreeObj(&func);
        return Ok;
    } else {
        rc = func->function->builtIn.func(lz, callArgV, state, argv, argNames);
    }

    FREE_A(callArgV, FreeLazyExpr);
    freeFunc:
    FreeObj(&func);
    return rc;
}

enum OpRetCode EvalVar(struct LazyExpr *lz, struct State *state) {
    struct ArgV *const argv = lz->argv;
    struct ArgNames *const argNames = lz->argNames;
    char *const name = lz->expression->var;

    for (size_t i = 0; i < argv->size; ++i) {
        if (strcmp(name, argNames->array[i]) == 0) {
            return GetLazyExprVal(ID(argv, i), state, &lz->value);
        }
    }
    for (size_t i = 0; i < CNT(state->builtins); ++i) {
        if (strcmp(name, ID(state->builtins, i).identifier) == 0) {
            return GetLazyExprVal(ID(state->builtins, i).value, state, &lz->value);
        }
    }
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        if (strcmp(name, ID(state->identifiers, i).identifier) == 0) {
            return GetLazyExprVal(ID(state->identifiers, i).value, state, &lz->value);
        }
    }
    return UndefinedArg;
}

enum OpRetCode EvalExpr(struct LazyExpr *lz, struct State *state) {
    enum OpRetCode rc;
    switch (lz->expression->expType) {
        case Call:
            return EvalCall(lz, state);
        case Const:
            rc = EvalExpr(lz->expression->object, state);
            if (rc == Ok) {
                struct LazyExpr *tmp;
                CPYREF(lz->expression->object, tmp);

                FreeExpr(&lz->expression);
                FREE_A(lz->argv, FreeLazyExpr);
                FREE_A(lz->argNames, SUB_FREE);

                CPYREF(tmp->expression, lz->expression);
                CPYREF(tmp->argv, lz->argv);
                CPYREF(tmp->argNames, lz->argNames);
                if (tmp->value != NULL)
                    CPYREF(tmp->value, lz->value);
                else
                    lz->value = NULL;
            }
            return rc;
        case Var:
            return EvalVar(lz, state);
    }
    return UnknownErr;
}
