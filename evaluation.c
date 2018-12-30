#include "evaluation.h"

enum OpRetCode GetLazyExprVal(
        struct LazyExpr *lazyExpr,
        struct ArgNames *argNames,
        struct Object **out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, argNames, &lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    *out = lazyExpr->value;
    return Ok;
}

enum OpRetCode EvalCall(
        struct CallArgV *callArgV,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct Object **out) {
    struct Object *func;

    enum OpRetCode rc = EvalExpr(callArgV->array[0], argv, argNames, &func);
    if (rc != Ok)
        return rc;
    if (func->type != Func)
        return ArgTypeMismatch;
    size_t paramsNum = callArgV->size;
    if (paramsNum - 1 != func->func->argc)
        return ArgNumberMismatch;
    struct ArgV *innerArgV = NewArgV(callArgV->size - 1);
    if (innerArgV == NULL)
        return AllocationFailure;
    for (size_t i = 0; i < paramsNum; ++i) {
        struct Expression *const curExpr = callArgV->array[i + 1];
        innerArgV->array[i] =
                (struct LazyExpr) {
                        .expression = curExpr,
                        .argv = argv,
                        .value = NULL
                };
    }
    rc = func->func->funcType == BuiltIn
         ? func->func->builtin(innerArgV, out)
         : EvalExpr(func->func->expression, innerArgV, argNames, out);
    free(innerArgV);
    return rc;
}

enum OpRetCode EvalExpr(
        struct Expression *expression,
        struct ArgV *argv,
        struct ArgNames *argNames,
        struct Object **out) {
    switch (expression->expType) {
        case Call:
            return EvalCall(expression->paramsV, argv, argNames, out);
        case Const:
            *out = expression->object;
            return Ok;
        case Var:
            for (size_t i = 0; i < argv->size; ++i) {
                if (strcmp(argNames->array[i], expression->var) == 0) {
                    return GetLazyExprVal(&argv->array[i], argNames, out);
                }
            }
            return UndefinedArg;
    }
}
