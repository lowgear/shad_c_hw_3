#include <stdlib.h>

#include "models.h"
#include "builtins.h"

struct ArgV emptyArgV = {.size = 0};

struct ArgNames emptyArgNames = {.size = 0};

void FreeExpr(struct Expression *expression) {
    FREEREF(expression);
    switch (expression->expType) {
        case Call:
            for (size_t i = 0; i < expression->paramsV->size; ++i) {
                FreeExpr(ID(expression->paramsV, i));
            }
            FREE_V(expression->paramsV);
            break;
        case Const:
            FreeObj(expression->object);
            break;
        case Var:
            free(expression->var);
            break;
    }
}

bool InitState(struct State *state) {
    INIT_VEC(state->builtins, 1, goto fail);
    for (size_t i = 0; i < BUILTINS_SIZE; ++i) {
        PUSH_BACK_P(&state->builtins, *BUILTINS[i], goto freeBuiltins);
    }
    INIT_VEC(state->identifiers, 1, goto freeBuiltins);

    return true;

    freeBuiltins:
    free(state->builtins);
    fail:
    return false;
}

void FreeState(struct State *state) {
    for (size_t i = 0; i < CNT(state->identifiers); ++i) {
        FreeObj(ID(state->identifiers, i).value);
        free(ID(state->identifiers, i).identifier);
    }
    FREE_V(state->identifiers);

    FREE_V(state->builtins);
}

void FreeObj(struct Object *object) {
    FREEREF(object);

    switch (object->type) {
        case Func:
            if (!object->function->isUserDefined)
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
            FreeLazyExpr(object->pair->first);
            FreeLazyExpr(object->pair->second);
            break;
        case Null:
            return;
    }
    free(object);
}

void FreeLazyExpr(struct LazyExpr *lazyExpr) {
    FREEREF(lazyExpr);

    FreeObj(lazyExpr->value);
    FreeExpr(lazyExpr->expression);
    for (size_t i = 0; i < SIZE(lazyExpr->argNames); ++i) {
        free(ID(lazyExpr->argNames, i));
    }
    FREE_V(lazyExpr->argNames);
    for (size_t i = 0; i < SIZE(lazyExpr->argv); ++i) {
        FreeLazyExpr(ID(lazyExpr->argv, i));
    }
    FREE_V(lazyExpr->argv);
}
