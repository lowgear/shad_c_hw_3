#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "evaluation.h"
#include "builtins.h"
#include "lisp_io.h"
#include "utils/error_check_tools.h"

enum ExitCodes {
    Fine = 0,
    WrongArgNum,
    FailedAlloc,
    FailOpen,
    FailSyntax,
    FailRuntime,
    FailIO
};

int main(int argc, char *argv[]) {
    enum ExitCodes rv = Fine;
    CHECK(argc == 2, "expected 1 argument", rv = WrongArgNum, exit);

    struct State state;
    CHECK(InitState(&state), "failed init state", rv = FailedAlloc, exit);

    FILE *f;
    char *const inFile = argv[1];
    CHECK_F(f = fopen(inFile, "r"), "open", inFile, rv, FailOpen, freeState);

    while (1) {
        struct Expression *expr;
        enum OpRetCode rc = ReadExpression(f, &expr);
        if (rc == eOf) break;
        CHK(rc == Ok, (void) 0, goto freeExpr);

        struct Object *res;
        rc = EvalExpr(expr, &emptyArgV, &emptyArgNames, &state, &res);
        CHK(rc == Ok, (void) 0, goto freeExpr);

        rc = WriteObject(stdout, &state, res);
        printf("\n");
        CHK(rc == Ok, (void) 0, goto freeObj);

        freeObj:
        FreeObj(&res);
        freeExpr:
        FreeExpr(&expr);

        CHECK(rc != IoError, "IO error", rv = FailIO, closeFile);
        CHECK(!(rc & RuntimeError), "runtime error", rv = FailRuntime, closeFile);
        CHECK(rc == Ok, "syntax error", rv = FailSyntax, closeFile);
    }

    closeFile:
    fclose(f);

    freeState:
    FreeState(&state);

    exit:
    return rv;
}
