#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "utils/vector.h"

struct Expression;
struct Object;
struct Function;
struct ArgV;
struct State;
struct Pair;
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
    BuiltIn,
    UserDef
};

struct Expression {
    union {
        struct CallParams *paramsV;
        struct Object *object;
        char *var;
    };
    enum ExpType expType;
};

struct UserDefFunc {
    struct ArgNames *head;
    struct Expression *body;
};

struct Function {
    union {
        struct UserDefFunc userDef;
        BuiltInFunc builtIn;
    };
    const char *name;
    enum FuncType type;
    uint8_t argc;
};

struct Pair {
    struct Object *first;
    struct Object *second;
};

struct Object {
    union {
        struct Function *function;
        int32_t integer;
        struct Pair pair;
    };
    enum Type type;
};

struct LazyExpr {
    struct Expression *expression;
    struct ArgV *argv;
    struct ArgNames *argNames;
    struct Object *value;
};

DEF_ARRAY(ArgV, struct LazyExpr);

struct IdentifierValuePair {
    char *identifier;
    struct Object *value;
};

DEF_VECTOR(IdentifierList, struct IdentifierValuePair)
DEF_VECTOR(ObjectList, struct Object*)

struct State {
    struct IdentifierList *builtins;
    struct IdentifierList *identifiers;
    struct ObjectList *objects;
};

extern struct ArgV emptyArgV;
extern struct ArgNames emptyArgNames;

bool InitState(struct State *state);

void FreeState(struct State *state);
void FreeExpr(struct Expression *expression);

void FreeObj(struct Object *object);
