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
        enum RetCode rc = ReadExpression(f, &expr);
        if (rc == eOf)
            break;
        CHECK(rc == IoOk, "failed", rv = FailSyntax, freeExpr); // todo message
        // todo io error

        struct Object *res;
        enum OpRetCode orc = EvalExpr(expr, &emptyArgV, &emptyArgNames, &state, &res);
        CHECK(!(orc & RuntimeError), "runtime error", rv = FailRuntime, freeExpr);
        CHECK(orc == Ok, "syntax error", rv = FailSyntax, freeExpr);

        WriteObject(stdout, res);
        printf("\n");

        FreeExpr(expr);
        continue;

        freeExpr:
        FreeExpr(expr);
        goto closeFile;
    }

    closeFile:
    fclose(f);
    freeState:
    FreeState(&state);
    exit:
    return rv;
}