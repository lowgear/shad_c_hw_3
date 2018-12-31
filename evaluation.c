#include "evaluation.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct State *state,
        const struct Object **out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, lazyExpr->argNames, state, &lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    *out = lazyExpr->value;
    return Ok;
}

enum OpRetCode EvalCall(
        struct CallArgV *callArgV,
        struct ArgV *argv,
        const struct ArgNames *argNames,
        struct State *state,
        const struct Object **out) {
    const struct Object *func;

    enum OpRetCode rc = EvalExpr(callArgV->array[0], argv, argNames, state, &func);
    if (rc != Ok)
        return rc;
    if (func->type != Func)
        return ArgTypeMismatch;
    size_t paramsNum = callArgV->size;
    if (paramsNum - 1 != func->func.argc)
        return ArgNumberMismatch;
    struct ArgV *innerArgV;
    INIT_ARR(innerArgV, callArgV->size - 1, return AllocationFailure);
    for (size_t i = 0; i < paramsNum - 1; ++i) {
        struct Expression *const curExpr = callArgV->array[i + 1];
        innerArgV->array[i] =
                (struct LazyExpr) {
                        .expression = curExpr,
                        .argv = argv,
                        .value = NULL
                };
    }
    rc = func->func.funcType == BuiltIn
         ? func->func.builtin(innerArgV, state, out)
         : EvalExpr(func->func.expression, innerArgV, argNames, state, out);
    free(innerArgV);
    return rc;
}

enum OpRetCode EvalExpr(
        const struct Expression *expression,
        struct ArgV *argv,
        const struct ArgNames *argNames,
        struct State *state,
        const struct Object **out) {
    switch (expression->expType) {
        case Call:
            return EvalCall(expression->paramsV, argv, argNames, state, out);
        case Const:
            *out = expression->object;
            return Ok;
        case Var:
            for (size_t i = 0; i < argv->size; ++i) {
                if (strcmp(argNames->array[i], expression->var) == 0) {
                    return GetLazyExprVal(&argv->array[i], state, out);
                }
            }
            for (size_t i = 0; i < state->identifiers->cnt; ++i) {
                if (strcmp(ID(state->identifiers, i).identifier, expression->var) == 0) {
                    *out = ID(state->identifiers, i).value;
                    return Ok;
                }
            }
            return UndefinedArg;
    }
}
