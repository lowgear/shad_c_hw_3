#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "evaluation.h"
#include "builtins.h"
#include "lisp_io.h"

#include <stdio.h>


int main(int argc, char *argv[]) {

//    int n = 0;
//    char s[100];
//    scanf("%*s%n%s", &n, s);
//    printf("%d\n%s", n, s);
//
//    return 0;

    if (argc != 2)
        return 1;
    FILE *f = fopen(argv[1], "r");

    struct State state;
    INIT_VEC(state.identifiers, 1, return 3);
    PUSH_BACK_P(&state.identifiers, define, return 6);

    struct Expression *expr;
    if (ReadExpression(f, &expr) != Ok)
        return 2;

    const struct Object *res;
    if (EvalExpr(expr, &emptyArgV, &emptyArgNames, &state, &res) != Ok)
        return 4;

    if (ReadExpression(f, &expr) != Ok)
        return 2;
    if (EvalExpr(expr, &emptyArgV, &emptyArgNames, &state, &res) != Ok)
        return 4;

    if (WriteObject(stdout, res) != IoOk)
        return 5;

    return 0;
}