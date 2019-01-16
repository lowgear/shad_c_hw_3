#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "utils/vector.h"
#include "utils/new_tools.h"
#include "utils/smartptr_tools.h"

enum OpRetCode {
    Ok = 0,
    eOf,
    IoError,
    UndefinedArg,
    IdentifierRedefinition,
    SyntaxViolation,
    ArgNumberMismatch,
    RuntimeError = 1 << 7,
    ArgTypeMismatch,
    AllocationFailure,
    DBZ,
    UnknownErr
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

struct Expression;
struct Object;
struct Function;
struct ArgV;
struct State;
struct Pair;
struct LazyExpr;
DEF_ARRAY(ArgNames, char *)
DEF_ARRAY(CallParams, struct Expression *)

typedef enum OpRetCode (*BuiltInFunc)(
        struct LazyExpr *lz,
        struct ArgV *argv,
        struct State *state,
        struct ArgV *localArgv,
        struct ArgNames *localAN);

struct Expression {
    union {
        struct CallParams *paramsV;
        struct LazyExpr *object;
        char *var;
    };
    REFCNT_DEF;
    enum ExpType expType : 2;
};

struct UserDefFunc {
    struct ArgNames *head;
    struct Expression *body;
};

struct _ {
    BuiltInFunc func;
    void *isUserDefined;
};

#define VARIADIC_ARGS UINT8_MAX

struct Function {
    union {
        struct UserDefFunc userDef;
        struct _ builtIn;
    };
    char *name;
    uint8_t argc;
};

struct Pair {
    struct LazyExpr *first;
    struct LazyExpr *second;
};

struct Object {
    union {
        struct Function *function;
        int32_t integer;
        struct Pair *pair;
    };
    REFCNT_DEF;
    enum Type type;
};

struct LazyExpr {
    struct Expression *expression;
    struct ArgV *argv;
    struct ArgNames *argNames;
    struct Object *value;
    REFCNT_DEF;
};

DEF_ARRAY(ArgV, struct LazyExpr*)

struct IdentifierValuePair {
    char *identifier;
    struct LazyExpr *value;
};

DEF_VECTOR(IdentifierList, struct IdentifierValuePair)

struct State {
    struct IdentifierList *builtins;
    struct IdentifierList *identifiers;
};

extern struct ArgV emptyArgV;
extern struct ArgNames emptyArgNames;

bool InitState(struct State *state);

void FreeState(struct State *state);

void FreeExpr(struct Expression **expression);

void FreeObj(struct Object **object);

void FreeLazyExpr(struct LazyExpr **lazyExpr);

#undef REFCNT_DEF
