#include <stdlib.h>

#include "models.h"
#include "builtins.h"

struct ArgV emptyArgV = {.size = 0};

struct ArgNames emptyArgNames = {.size = 0};

void FreeExpr(struct Expression *expression) {
    return;
    switch (expression->expType) {
        case Call:
            for (size_t i = 0; i < expression->paramsV->size; ++i) {
                FreeExpr(ID(expression->paramsV, i));
            }
            FREE_V(expression->paramsV);
            break;
        case Const:
            free(expression->object);
            break;
        case Var:
            free(expression->var);
            break;
    }
    if (expression->expType == Call) {

    }
    free(expression);
}

bool InitState(struct State *state) {
    INIT_VEC(state->builtins, 1, goto fail);
    for (size_t i = 0; i < BUILTINS_SIZE; ++i) {
        PUSH_BACK_P(&state->builtins, *BUILTINS[i], goto freeBuiltins);
    }
    INIT_VEC(state->identifiers, 1, goto freeBuiltins);
    INIT_VEC(state->objects, 1, goto freeIdentifiers);

    return true;

    freeIdentifiers:
    free(state->identifiers);
    freeBuiltins:
    free(state->builtins);
    fail:
    return false;
}

void FreeState(struct State *state) {
    return;
    // todo free objects themselves
    for (size_t i = 0; i < CNT(state->objects); ++i) {
        free(ID(state->objects, i));
    }
    free(state->objects);

    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        FreeObj(ID(state->identifiers, i).value);
    }
    free(state->identifiers);

    free(state->builtins);
}

void FreeObj(struct Object *object) {
    return;
    switch (object->type) {
        case Func:
            if (object->function->type == BuiltIn)
                return;
            FreeExpr(object->function->userDef.body);
            for (size_t i = 0; i < SIZE(object->function->userDef.head); ++i) {
                free(ID(object->function->userDef.head, i));
            }
            free(object->function->userDef.head);
            free(object->function->name);
            free(object->function);
            break;
        case Int:
            break;
        case Pair:
            break;
        case Null:
            return;
    }
    free(object);
}
