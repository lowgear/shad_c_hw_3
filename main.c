#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "utils/vector.h"

struct Expression;

enum OpRetCode {
    Ok,
    ArgNumberMismatch,
    ArgTypeMismatch,
    AllocationFailure,
    DBZ
};

enum Type {
    Func,
    Int,
    Pair,
    Null,
};

enum ExpType {
    Call,
    Const,
    Var
};

DEF_VECTOR(CallArgV, struct Expression *);

struct Expression {
    union {
        struct CallArgV *paramsV;
        struct Object *object;
        size_t varId;
    };
    enum ExpType expType;
};

struct Function {
    size_t argc;
    struct Expression *expression;
};

struct Pair {
    struct Object *first;
    struct Object *second;
};

struct Object {
    enum Type type;
    union {
        struct Function *func;
        int32_t integer;
        struct Pair *pair;
    };
};

struct LazyExpr {
    struct Expression *expression;
    struct ArgV *argv;
    struct Object *value;
};

enum OpRetCode EvalExpr(struct Expression *expression, struct ArgV *argv, struct Object **out);

enum OpRetCode GetLazyExprVal(struct LazyExpr *lazyExpr, struct Object **out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, &lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    *out = lazyExpr->value;
    return Ok;
}

DEF_VECTOR(ArgV, struct LazyExpr);

enum OpRetCode EvalCall(struct CallArgV *callArgV, struct ArgV *argv, struct Object **out) {
    struct Object *func;

    enum OpRetCode rc = EvalExpr(callArgV->array[0], argv, &func);
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
                curExpr->expType == Var
                ? argv->array[curExpr->varId]
                : (struct LazyExpr) {
                        .expression = curExpr,
                        .argv = argv,
                        .value = NULL
                };
    }
    rc = EvalExpr(func->func->expression, innerArgV, out);
    free(innerArgV);
    return rc;
}

enum OpRetCode EvalExpr(struct Expression *expression, struct ArgV *argv, struct Object **out) {
    switch (expression->expType) {
        case Call:
            return EvalCall(expression->paramsV, argv, out);
        case Const:
            *out = expression->object;
            return Ok;
        case Var:
            return GetLazyExprVal(&argv->array[expression->varId], out);
    }
}

int main() {


    return 0;
}