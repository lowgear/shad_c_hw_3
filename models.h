#pragma once

#include <inttypes.h>

#include "utils/vector.h"

struct Expression;

enum OpRetCode {
    Ok,
    ArgNumberMismatch,
    ArgTypeMismatch,
    AllocationFailure,
    UndefinedArg,
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
        const char *var;
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

struct Arg {
    const char *name;
    struct LazyExpr lazyExpr;
};

DEF_VECTOR(ArgV, struct Arg);
