#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"
#pragma ide diagnostic ignored "cert-err58-cpp"
#include "gtest/gtest.h"

extern "C" {
#include "models.h"
}

#define TEST_FREES \
TEST_FREE(FreeObj, Object) \
TEST_FREE(FreeExpr, Expression) \
TEST_FREE(FreeLazyExpr, LazyExpr)

#define TEST_FREE(FreeFunc, T) \
TEST(FreeFunc, CASE) { \
    T object; \
    object.refCnt = 2; \
    T *ref = &object; \
\
    FreeFunc(&ref); \
\
    CUR_CHECK; \
}

#define CUR_CHECK EXPECT_EQ(object.refCnt, 1);
#define CASE DecreaseRefCnt

TEST_FREES

#undef CUR_CHECK
#undef CASE

#define CUR_CHECK EXPECT_EQ(ref, nullptr);
#define CASE MakeRefNull

TEST_FREES

#undef CUR_CHECK
#undef CASE

#undef TEST_FREE

TEST(emptyArgV, refCnt_is_not_0) {
    EXPECT_GT(emptyArgV.refCnt, 0);
}

TEST(emptyArgNames, refCnt_is_not_0) {
    EXPECT_GT(emptyArgNames.refCnt, 0);
}

TEST(OpRetCode, Types) {
#define CHECK_IS_RUNTIME_ERR(code) EXPECT_TRUE(code & RuntimeError)
#define CHECK_IS_NOT_RUNTIME_ERR(code) EXPECT_FALSE(code & RuntimeError)
    CHECK_IS_RUNTIME_ERR(ArgTypeMismatch);
    CHECK_IS_RUNTIME_ERR(AllocationFailure);
    CHECK_IS_RUNTIME_ERR(DBZ);
    CHECK_IS_RUNTIME_ERR(UnknownErr);

    CHECK_IS_NOT_RUNTIME_ERR(Ok);
    CHECK_IS_NOT_RUNTIME_ERR(eOf);
    CHECK_IS_NOT_RUNTIME_ERR(IoError);
    CHECK_IS_NOT_RUNTIME_ERR(UndefinedArg);
    CHECK_IS_NOT_RUNTIME_ERR(IdentifierRedefinition);
    CHECK_IS_NOT_RUNTIME_ERR(SyntaxViolation);
    CHECK_IS_NOT_RUNTIME_ERR(ArgNumberMismatch);

#undef CHECK_IS_RUNTIME_ERR
#undef CHECK_IS_NOT_RUNTIME_ERR
}

#pragma clang diagnostic pop