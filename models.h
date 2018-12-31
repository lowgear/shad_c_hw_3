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
    IdentifierRedefinition,
    SyntaxViolation,
    DBZ,
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

DEF_ARRAY(CallArgV, struct Expression *);

DEF_ARRAY(ArgNames, const char *)

struct Expression {
    union {
        struct CallArgV *paramsV;
        struct Object *object;
        const char *var;
    };
    enum ExpType expType;
};

struct ArgV;

struct State;

struct Function {
    union {
        const struct Expression *expression;

        enum OpRetCode (*builtin)(
                struct ArgV *argv,
                struct State *state,
                const struct Object **out);
    };

    const char *name;

    uint8_t argc;

    enum FuncType funcType;
};

struct Pair {
    const struct Object *first;
    const struct Object *second;
};

struct Object {
    union {
        struct Function func;
        int32_t integer;
        struct Pair pair;
    };
    enum Type type;
};

struct LazyExpr {
    const struct Expression *expression;
    struct ArgV *argv;
    const struct ArgNames *argNames;
    const struct Object *value;
};

DEF_ARRAY(ArgV, struct LazyExpr);

struct IdentifierValuePair {
    const char *identifier;
    const struct Object *value;
};

DEF_VECTOR(IdentifierList, struct IdentifierValuePair)

struct State {
    struct IdentifierList *identifiers;
};

struct ArgV emptyArgV;

struct ArgNames emptyArgNames;

void FreeExpr(const struct Expression *expression);
