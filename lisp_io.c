#include <string.h>
#include <stdlib.h>

#include "lisp_io.h"
#include "evaluation.h"
#include "utils/vector.h"
#include "utils/goodies.h"
#include "utils/iotools.h"
#include "utils/strtools.h"

#define CHK_FERROR(onFail) if (ferror(file)) onFail;

DEF_VECTOR(ExprList, struct Expression*)

enum OpRetCode ReadCall(FILE *file, struct Expression **out) {
    enum OpRetCode rc = Ok;
    struct ExprList *exprList;
    INIT_VEC(exprList, 1, return AllocationFailure);
    while (1) {
        // check for end of call
        int n = 0;
        fscanf(file, " )%n", &n);
        CHK_FERROR(return IoError);
        if (n)
            break;

        struct Expression *curExp;
        rc = ReadExpression(file, &curExp);
        if (rc != Ok)
            goto freeExprList;
        PUSH_BACK_P(&exprList, curExp, goto freeCurExpr);
        continue;

        freeCurExpr:
        FreeExpr(&curExp);
        goto freeExprList;
    }

    struct Expression *res;
    NEWSMRT(res, struct Expression, goto freeExprList);
    res->expType = Call;
    INIT_ARR(res->paramsV, CNT(exprList), goto freeCurExpr);
    for (size_t i = 0; i < exprList->cnt; ++i) {
        res->paramsV->array[i] = ID(exprList, i);
    }
    free(exprList);
    *out = res;
    return Ok;

    freeExprList:
    for (size_t i = 0; i < CNT(exprList); ++i) {
        FreeExpr(&ID(exprList, i));
    }
    free(exprList);
    return rc;
}

enum OpRetCode HandleReadInt(char *str, int32_t value, struct Expression **out) {
    (*out)->expType = Const;
    NEWSMRT((*out)->object, struct Object, goto freeOut);
    (*out)->object->type = Int;
    (*out)->object->integer = value;
    free(str);
    return Ok;

    freeOut:
    free(*out);
    free(str);

    return AllocationFailure;
}

enum OpRetCode HandleReadVar(char *str, struct Expression **out) {
    (*out)->expType = Var;
    (*out)->var = str;
    return Ok;
}

enum OpRetCode ReadExpression(FILE *file, struct Expression **out) {
    // check for EOF
    fscanf(file, " ");
    if (feof(file))
        return eOf;
    CHK_FERROR(return IoError);

    enum OpRetCode rc = Ok;
    // check if next expression is Call and skip open bracket
    int n = 0;
    fscanf(file, " (%n", &n);
    CHK_FERROR(return IoError);
    if (n) {
        return ReadCall(file, out);
    }

    size_t tokenLen = NextStrLen(file);
    CHK_FERROR(return IoError);
    char *str = NEWARR(char, tokenLen + 1);
    if (str == NULL)
        return AllocationFailure;

    // read token without whitespaces or brackets
    fscanf(file, "%[^\t\n\v\f\r ()]", str);
    CHK_FERROR(goto freeStr);

    NEWSMRT(*out, struct Expression, goto freeStr);

    int32_t value;
    if (ParseInt32(str, &value)) {
        return HandleReadInt(str, value, out);
    }
    return HandleReadVar(str, out);

    freeStr:
    free(str);
    return rc;
}

enum OpRetCode WriteObject(FILE *file, struct State *state, const struct Object *object) {
    switch (object->type) {
        case Func:
            fprintf(file, "%s", object->function->name);
            break;
        case Int:
            fprintf(file, "%d", object->integer);
            break;
        case Pair:
            fprintf(file, "(cons ");
            CHK_FERROR(return IoError);
            struct Object *out;
            enum OpRetCode rc;

#define HANDLE_PART(part) do { \
                rc = GetLazyExprVal(object->pair->part, state, &out); \
                if (rc != Ok) return rc; \
                rc = WriteObject(file, state, out); \
                FreeObj(&out); \
                if (rc != Ok) return rc; \
            } while (0)

            HANDLE_PART(first);
            fprintf(file, " ");
            CHK_FERROR(return IoError);
            HANDLE_PART(second);

#undef HANDLE_PART

            fprintf(file, ")");
            break;
        case Null:
            fprintf(file, "null");
            break;
        default:
            return UnknownErr;
    }
    CHK_FERROR(return IoError);
    return Ok;
}
