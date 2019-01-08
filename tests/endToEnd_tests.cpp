#include "gtest/gtest.h"
#include <stdio.h>
#include <string.h>

extern "C" {
#include "evaluation.h"
#include "builtins.h"
#include "lisp_io.h"
#include "utils/error_check_tools.h"
}

void RunEndToEnd(FILE *in, FILE *out) {
    State state;
    ASSERT_TRUE(InitState(&state));

    while (true) {
        struct Expression *expr;
        enum OpRetCode rc = ReadExpression(in, &expr);
        if (rc == eOf) break;
        CHK(rc == Ok, (void) 0, goto freeExpr);

        struct Object *res;
        rc = EvalExpr(expr, &emptyArgV, &emptyArgNames, &state, &res);
        CHK(rc == Ok, (void) 0, goto freeExpr);

        rc = WriteObject(out, &state, res);
        CHK(rc == Ok, (void) 0, goto freeObj);
        fprintf(out, "\n");
        CHK(!ferror(in), (void) 0, goto freeObj);

        freeObj:
        FreeObj(&res);
        freeExpr:
        FreeExpr(&expr);

        CHK(rc == Ok, (void) 0, goto freeState);
    }

    freeState:
    FreeState(&state);
}

TEST(EndToEnd, Sanity) {
    char in[] = "1\n"
                "(+ 1 1)\n"
                "(- 1 1)\n"
                "(* 1 1)\n"
                "(/ 1 1)\n"
                "(% 1 1)\n"
                "(= 1 1)\n"
                "(< 1 1)\n"
                "(> 1 1)\n"
                "(define (f) 1)\n"
                "(f)\n"
                "(cons 1 null)\n"
                "(define (d x) x)\n"
                "(d 4)\n"
                "(d (cons 5 6))\n"
                "(define (fact n) (if (= n 1) 1 (* n (fact (- n 1)))))\n"
                "(fact 5)\n"
                "(/ 6 2)\n"
                "(% -7 2)\n"
                "(car (cons 1 (cons 2 (cons 3 null))))\n"
                "(define (N) 5)\n"
                "(define (fibNimpl pprev prev n) (if (= n 0) null (cons (+ pprev prev) (fibNimpl prev (+ pprev prev) (- n 1)))))\n"
                "(define (fibN n) (cons 1 (cons 1 (fibNimpl 1 1 (- n 2)))))\n"
                "(fibN (N))\n"
                "\n"
                "(define (natural_impl x) (cons x (natural_impl (+ x 1))))\n"
                "(define (natural) (natural_impl 1))\n"
                "(car (cdr (natural)))\n"
                "\n"
                "(define (x) 0)\n"
                "(if (= (x) 0) 1 (1 / (x)))\n"
                "\n"
                "(let kek 5)\n"
                "kek\n"
                "\n"
                "(let sum (lambda (a b) (+ a b)))\n"
                "(sum 1 2)\n"
                "(lambda (a b) (+ a b))\n"
                "sum";
    const size_t size = 10000;
    char out[size];
    out[size - 1] = '\0';

    FILE *inFile = fmemopen(in, sizeof(in), "r");
    ASSERT_NE(inFile, nullptr);
    FILE *outFile = fmemopen(out, size, "w");
    RunEndToEnd(inFile, outFile);
    ASSERT_EQ(fclose(inFile), 0);
    ASSERT_EQ(fclose(outFile), 0);

    ASSERT_EQ(out[size - 1], '\0');

    EXPECT_TRUE(strcmp(out, "1\n"
                            "2\n"
                            "0\n"
                            "1\n"
                            "1\n"
                            "0\n"
                            "1\n"
                            "0\n"
                            "0\n"
                            "function\n"
                            "1\n"
                            "(cons 1 null)\n"
                            "function\n"
                            "4\n"
                            "(cons 5 6)\n"
                            "function\n"
                            "120\n"
                            "3\n"
                            "-1\n"
                            "1\n"
                            "function\n"
                            "function\n"
                            "function\n"
                            "(cons 1 (cons 1 (cons 2 (cons 3 (cons 5 null)))))\n"
                            "function\n"
                            "function\n"
                            "2\n"
                            "function\n"
                            "1\n"
                            "variable\n"
                            "5\n"
                            "variable\n"
                            "3\n"
                            "<lambda function>\n"
                            "<lambda function>\n") == 0);
}