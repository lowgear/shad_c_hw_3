#include <stdlib.h>
#include <assert.h>

#include "models.h"
#include "builtins.h"

struct ArgV emptyArgV = {.size = 0, .refCnt = 1};

struct ArgNames emptyArgNames = {.size = 0, .refCnt = 1};

void FreeExpr(struct Expression **expressionP) {
    struct Expression *expression = *expressionP;
    FREEREF_RET(expression);
    switch (expression->expType) {
        case Call:
            for (size_t i = 0; i < SIZE(expression->paramsV); ++i) {
                FreeExpr(&ID(expression->paramsV, i));
            }
            FREE_V(&expression->paramsV);
            break;
        case Const:
            FreeLazyExpr(&expression->object);
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
        PUSH_BACK_P(&state->builtins, *BUILTINS[i], goto freeBuiltins;);
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
        FreeLazyExpr(&ID(state->identifiers, i).value);
        free(ID(state->identifiers, i).identifier);
    }
    FREE_V(&state->identifiers);

    FREE_V(&state->builtins);
}

void FreeObj(struct Object **objectP) {
    struct Object *object = *objectP;
    FREEREF_RET(object);

    switch (object->type) {
        case Func:
            if (!object->function->builtIn.isUserDefined)
                return;
            FreeExpr(&object->function->userDef.body);

            FREE_A(object->function->userDef.head, SUB_FREE);

            if (object->function->name != NULL)
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

    FREE_A(lazyExpr->argNames, SUB_FREE);
    struct LazyExpr *freeingHead = lazyExpr;

    for (struct LazyExpr *i = lazyExpr; i != NULL; i = (struct LazyExpr *) i->argNames) {
        if (i->argv == NULL || --REFCNT(i->argv) != 0) {
            i->argv = NULL;
            continue;
        }
        for (size_t j = 0; j < SIZE(i->argv); ++j) {
            if (--REFCNT(ID(i->argv, j)) == 0) {
                freeingHead =
                        (struct LazyExpr *) (
                                freeingHead->argNames = (struct ArgNames *) ID(i->argv, j));
                FREE_A(freeingHead->argNames, SUB_FREE);
            }
        }
    }

    while (lazyExpr != NULL) {
        FreeExpr(&lazyExpr->expression);
        FreeObj(&lazyExpr->value);
        if (lazyExpr->argv != NULL && REFCNT(lazyExpr->argv) == 0)
            free(lazyExpr->argv);

        struct LazyExpr *const next = (struct LazyExpr *const) lazyExpr->argNames;
        free(lazyExpr);
        lazyExpr = next;
    }

    *lazyExprP = NULL;
}
