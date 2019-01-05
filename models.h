#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "utils/vector.h"
#include "utils/goodies.h"
#include "utils/smartptr_tools.h"

struct Expression;
struct Object;
struct Function;
struct ArgV;
struct State;
struct Pair;
struct LazyExpr;
DEF_ARRAY(ArgNames, char *);
DEF_ARRAY(CallParams, struct Expression *);

typedef enum OpRetCode (*BuiltInFunc)(
        struct ArgV *argv,
        struct State *state,
        struct Object **out);

enum OpRetCode {
    Ok = 0,
    UndefinedArg,
    IdentifierRedefinition,
    SyntaxViolation,
    ArgNumberMismatch,
    RuntimeError = 1 << 6,
    ArgTypeMismatch = RuntimeError,
    AllocationFailure,
    DBZ,
    eOf,
    IoError,
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

struct Expression {
    union {
        struct CallParams *paramsV;
        struct Object *object;
        char *var;
    };
    REFCNT_DEF;
    enum ExpType expType : 2;
};

struct UserDefFunc {
    struct ArgNames *head;
    struct Expression *body;
};

struct Function {
    union {
        struct UserDefFunc userDef;
        struct {
            BuiltInFunc builtIn;
            void *isUserDefined;
        };
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

DEF_ARRAY(ArgV, struct LazyExpr*);

struct IdentifierValuePair {
    char *identifier;
    struct Object *value;
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
