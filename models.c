#include <stdlib.h>
#include <assert.h>

#include "models.h"
#include "builtins.h"

struct ArgV emptyArgV = {.size = 0};

struct ArgNames emptyArgNames = {.size = 0};

void FreeExpr(struct Expression **expressionP) {
    struct Expression *expression = *expressionP;
    FREEREF_RET(expression);
    switch (expression->expType) {
        case Call:
            for (size_t i = 0; i < expression->paramsV->size; ++i) {
                FreeExpr(&ID(expression->paramsV, i));
            }
            FREE_V(expression->paramsV);
            break;
        case Const:
            FreeObj(&expression->object);
            break;
        case Var:
            free(expression->var);
            break;
    }
    free(expression);
    *expressionP = NULL;
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
        FreeObj(&ID(state->identifiers, i).value);
    }
    FREE_V(state->identifiers);

    FREE_V(state->builtins);
}

void FreeObj(struct Object **objectP) {
    struct Object *object = *objectP;
    FREEREF_RET(object);

    switch (object->type) {
        case Func:
            if (!object->function->isUserDefined)
                return;
            FreeExpr(&object->function->userDef.body);

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
            FreeLazyExpr(&object->pair->first);
            FreeLazyExpr(&object->pair->second);
            free(object->pair);
            break;
        case Null:
            *objectP = NULL;
            return;
    }
    free(object);
    *objectP = NULL;
}

void FreeLazyExpr(struct LazyExpr **lazyExprP) {
    struct LazyExpr *lazyExpr = *lazyExprP;
    FREEREF_RET(lazyExpr);

    if (lazyExpr->value != NULL) {
        FreeObj(&lazyExpr->value);
    }
    FreeExpr(&lazyExpr->expression);
    free(lazyExpr);
    *lazyExprP = NULL;
}
