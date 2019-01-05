#include "evaluation.h"
#include "utils/goodies.h"
#include "utils/error_check_tools.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        struct Object **out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, lazyExpr->argNames, state, &lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    CPYREF(lazyExpr->value, *out);
    return Ok;
}

enum OpRetCode EvalVarLazy(struct Expression *expr, struct ArgV *argv, struct ArgNames *argNames, struct State *state,
                           struct LazyExpr **out) {
    char *name = expr->var;
    for (size_t i = 0; i < CNT(state->builtins); ++i) {
        if (strcmp(name, ID(state->builtins, i).identifier) == 0) {
            NEWSMRT(*out, struct LazyExpr, return AllocationFailure);
            CPYREF(expr, (*out)->expression);
            (*out)->argNames = argNames;
            CPYREF(argv, (*out)->argv);
            CPYREF(ID(state->builtins, i).value, (*out)->value);
            return Ok;
        }
    }
    for (size_t i = 0; i < argv->size; ++i) {
        if (strcmp(name, argNames->array[i]) == 0) {
            CPYREF(ID(argv, i), *out);
            return Ok;
        }
    }
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        if (strcmp(name, ID(state->identifiers, i).identifier) == 0) {
            NEWSMRT(*out, struct LazyExpr, return AllocationFailure);
            CPYREF(expr, (*out)->expression);
            (*out)->argNames = argNames;
            CPYREF(argv, (*out)->argv);
            CPYREF(ID(state->identifiers, i).value, (*out)->value);
            return Ok;
        }
    }
    return UndefinedArg;
}

enum OpRetCode MakeCallArgV(
        struct CallParams *callParams,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct ArgV **callArgV) {
    enum OpRetCode rc;

    INIT_ARR(*callArgV, callParams->size - 1, return AllocationFailure);
    for (size_t i = 1; i < callParams->size; ++i) {
        // shortcuts for Vars
        if (ID(callParams, i)->expType == Var) {
            rc = EvalVarLazy(ID(callParams, i), argv, argNames, state, &ID(*callArgV, i - 1));
            if (rc != Ok) {
                for (size_t j = 1; j < i; ++j) {
                    FreeLazyExpr(&ID(*callArgV, j - 1));
                }
                free(*callArgV);
                return rc;
            }
            continue;
        }

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

enum OpRetCode EvalCall(
        struct CallParams *callParams,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct Object **out) {
    if (callParams->size < 1)
        return SyntaxViolation;

    enum OpRetCode rc;
    struct Object *func;
    rc = EvalExpr(ID(callParams, 0), argv, argNames, state, &func);
    if (rc != Ok)
        return rc;
    CHK(func->type == Func, rc = ArgTypeMismatch, goto freeFunc)

    CHK(callParams->size - 1 == func->function->argc,
        rc = ArgNumberMismatch,
        goto freeFunc)

    struct ArgV *callArgV;
    rc = MakeCallArgV(callParams, argv, argNames, state, &callArgV);
    CHK(rc == Ok, (void) 0, goto freeCallArgV);

    rc = func->function->isUserDefined
         ? EvalExpr(func->function->userDef.body, callArgV, func->function->userDef.head, state, out)
         : func->function->builtIn(callArgV, state, out);

    freeCallArgV:
    FREE_A(callArgV, FreeLazyExpr);
    freeFunc:
    FreeObj(&func);
    return rc;
}

enum OpRetCode EvalVar(
        const char *name,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct Object **out) {
    for (size_t i = 0; i < CNT(state->builtins); ++i) {
        if (strcmp(name, ID(state->builtins, i).identifier) == 0) {
            CPYREF(ID(state->builtins, i).value, *out);
            return Ok;
        }
    }
    for (size_t i = 0; i < argv->size; ++i) {
        if (strcmp(name, argNames->array[i]) == 0) {
            return GetLazyExprVal(argv->array[i], state, out);
        }
    }
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        if (strcmp(name, ID(state->identifiers, i).identifier) == 0) {
            CPYREF(ID(state->identifiers, i).value, *out);
            return Ok;
        }
    }
    return UndefinedArg;
}

enum OpRetCode EvalExpr(
        struct Expression *expression,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct Object **out) {
    switch (expression->expType) {
        case Call:
            return EvalCall(expression->paramsV, argv, argNames, state, out);
        case Const:
            CPYREF(expression->object, *out);
            return Ok;
        case Var:
            return EvalVar(expression->var, argv, argNames, state, out);
    }
    return UnknownErr;
}
