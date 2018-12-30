#include <stdio.h>
#include <inttypes.h>

#include "utils/vector.h"

struct Expression;

enum OpRetCode {
    Ok,
    ArgNumberMismatch,
    ArgTypeMismatch,
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
        struct CallArgV *callArgV;
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

enum OpRetCode EvalExpr(struct Expression *expression, struct ArgV *argv, struct Object *out);

enum OpRetCode GetLazyExprVal(struct LazyExpr *lazyExpr, struct Object *out) {
    if (lazyExpr->value == NULL) {
        enum OpRetCode rc = EvalExpr(lazyExpr->expression, lazyExpr->argv, lazyExpr->value);
        if (rc != Ok)
            return rc;
    }
    out = lazyExpr->value;
    return Ok;
}

DEF_VECTOR(ArgV, struct LazyExpr*);

enum OpRetCode Invoke(struct Function *function, struct ArgV *argv, struct Object *out) {

}

enum OpRetCode EvalExpr(struct Expression *expression, struct ArgV *argv, struct Object *out) {
    struct Object *func;
    enum OpRetCode rc;
    switch (expression->expType) {
        case Call:
            rc = EvalExpr(expression->callArgV->array[0], argv, func);
            if (rc != Ok)
                return rc;

        case Const:
            out = expression->object;
            return Ok;
        case Var:
            return GetLazyExprVal(argv->array[expression->varId], out);
    }
}

int main() {


    return 0;
}