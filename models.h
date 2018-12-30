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

enum FuncType {
    UserDefined,
    BuiltIn
};

DEF_VECTOR(CallArgV, struct Expression *);

DEF_VECTOR(ArgNames, const char *)

struct Expression {
    union {
        struct CallArgV *paramsV;
        struct Object *object;
        const char *var;
    };
    enum ExpType expType;
};

struct ArgV;

struct Function {
    size_t argc;

    union {
        struct Expression *expression;

        enum OpRetCode (*builtin)(struct ArgV *argv, struct ArgNames *argNames, struct Object **out);
    };

    enum FuncType funcType;
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
        struct Pair pair;
    };
};

struct LazyExpr {
    struct Expression *expression;
    struct ArgV *argv;
    struct Object *value;
};

DEF_VECTOR(ArgV, struct LazyExpr);
