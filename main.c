#include <stdio.h>
#include <inttypes.h>

struct Expression;

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

struct Call {
    struct Expression *args;
};

struct Expression {
    union {
        struct Call *call;
        struct Object *object;
        size_t varId;
    };
    enum ExpType expType;
};

struct Function {
    char *name;
    size_t argc;
    struct Expression *expression;
};

struct Pair {
    struct Object *first;
    struct Object *second;
};

struct Call {
    struct Object *args;
};

struct Object {
    enum Type type;
    union {
        struct Function *func;
        int32_t integer;
        struct Pair *pair;
    };
};

struct Object *RunCall(struct Call *call) {

}

int main() {


    return 0;
}