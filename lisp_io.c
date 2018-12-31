#include <string.h>
#include <stdlib.h>

#include "lisp_io.h"
#include "utils/vector.h"
#include "utils/goodies.h"
#include "utils/iotools.h"
#include "utils/strtools.h"

DEF_VECTOR(ExprList, struct Expression*)

enum RetCode ReadCall(FILE *file, struct Expression **out) {
    enum RetCode rc = IoOk;
    struct ExprList *exprList;
    INIT_VEC(exprList, 1, return AllocFail);
    while (1) {
        int n = 0;
        fscanf(file, " )%n", &n);
        if (n)
            break;
        struct Expression *curExp;
        rc = ReadExpression(file, &curExp);
        if (rc != IoOk)
            goto freeExprList;
        PUSH_BACK_P(&exprList, curExp, goto freeCurExpr);
        continue;

        freeCurExpr:
        FreeExpr(curExp);
        goto freeExprList;
    }

    struct Expression *res = NEW(struct Expression);
    res->expType = Call;
    INIT_ARR(res->paramsV, exprList->cnt, goto freeCurExpr);
    for (size_t i = 0; i < exprList->cnt; ++i) {
        res->paramsV->array[i] = ID(exprList, i);
    }
    free(exprList);
    *out = res;
    return IoOk;

    freeExprList:
    for (size_t i = 0; i < exprList->cnt; ++i) {
        FreeExpr(ID(exprList, i));
    }
    free(exprList);
    return rc;
}

enum RetCode ReadExpression(FILE *file, struct Expression **out) {
    enum RetCode rc = IoOk;
    int n = 0;
    fscanf(file, " (%n", &n);
    if (n) {
        return ReadCall(file, out);
    }

    size_t tokenLen = NextStrLen(file);
    char *str = malloc(sizeof(char) * (tokenLen + 1));
    if (str == NULL)
        return AllocFail;
    fscanf(file, "%[^ ()]", str);
    // todo
    int32_t value;
    *out = NEW(struct Expression);
    if (*out == NULL)
        goto freeStr;
    if (ParseInt32(str, &value)) {
        **out = (struct Expression) {
                .expType = Const,
                .object = NEW(struct Object)
        };
        if ((*out)->object == NULL)
            goto freeOut;
        *(*out)->object = (struct Object) {
                .type = Int,
                .integer = value
        };
        return IoOk;
    }

    **out = (struct Expression) {
            .expType = Var,
            .var = str
    };

    return IoOk;

    freeOut:
    free(*out);
    freeStr:
    free(str);
    return rc;
}

enum RetCode WriteObject(FILE *file, const struct Object *object) {
    switch (object->type) {
        case Func:
            fprintf(file, "%s", object->func.name);
            break;
        case Int:
            fprintf(file, "%d", object->integer);
            break;
        case Pair:
            fprintf(file, "(cons ");
            enum RetCode rc = WriteObject(file, object->pair.first);
            if (rc != IoOk)
                return rc;
            fprintf(file, " ");
            rc = WriteObject(file, object->pair.first);
            fprintf(file, ")");
            break;
        case Null:
            fprintf(file, "null");
            break;
    }
    return IoOk;
}
