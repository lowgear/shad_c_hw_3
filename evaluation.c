#include "evaluation.h"
#import "impl_goodies.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        struct Object **out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, lazyExpr->argNames, state, &lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    *out = lazyExpr->value;
    return Ok;
}

enum OpRetCode MakeCallArgV(struct CallParams *callParams, struct ArgV *argv, struct ArgV **callArgV) {
    INIT_ARR(*callArgV, callParams->size - 1, return AllocationFailure);
    for (size_t i = 1; i < callParams->size; ++i) {
        (*callArgV)->array[i] =
                (struct LazyExpr) {
                        .expression = callParams->array[i + 1],
                        .argv = argv,
                        .value = NULL
                };
    }
    return Ok;
}

enum OpRetCode EvalCall(
        struct CallParams *callParams,
        struct ArgV *argv,
        struct State *state,
        struct Object **out) {
    if (callParams->size < 1)
        return SyntaxViolation;

    enum OpRetCode rc;
    GETARGT(func, 0, Func);

    if (callParams->size - 1 != func->function.argc)
        return ArgNumberMismatch;

    struct ArgV *callArgV;
    rc = MakeCallArgV(callParams, argv, &callArgV);
    if (rc != Ok)
        return rc;
    rc = func->function.type == BuiltIn
         ? func->function.builtIn(callArgV, state, out)
         : EvalExpr(func->function.userDef->body, callArgV, func->function.userDef->head, state, out);
    free(callArgV);
    return rc;
}

enum OpRetCode EvalVar(
        const char *name,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct State *state,
        struct Object **out) {
    for (size_t i = 0; i < argv->size; ++i) {
        if (strcmp(name, argNames->array[i]) == 0) {
            return GetLazyExprVal(&argv->array[i], state, out);
        }
    }
    for (size_t i = 0; i < CNT(state->builtins); ++i) {
        if (strcmp(name, ID(state->builtins, i).identifier) == 0) {
            *out = ID(state->builtins, i).value;
            return Ok;
        }
    }
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        if (strcmp(name, ID(state->identifiers, i).identifier) == 0) {
            *out = ID(state->identifiers, i).value;
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
            return EvalCall(expression->paramsV, argv, state, out);
        case Const:
            *out = expression->object;
            return Ok;
        case Var:
            return EvalVar(expression->var, argv, argNames, state, out);
    }
}
