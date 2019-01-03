#include "models.h"
#include "builtins.h"

struct ArgV emptyArgV = {.size = 0};

struct ArgNames emptyArgNames = {.size = 0};

void FreeExpr(struct Expression *expression) {
    if (expression->expType == Call) {
        for (size_t i = 0; i < expression->paramsV->size; ++i) {
            FreeExpr(ID(expression->paramsV, i));
        }
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
    // todo free objects themselves
    free(state->objects);
    free(state->identifiers);
    free(state->builtins);
}
