#include "gtest/gtest.h"
#include "utils/vector.h"

DEF_VECTOR(IntV, int)

TEST(Vector, Sanity) {
    IntV_Ptr vec;
    INIT_VEC(vec, 1, goto fault)

    for (int i = 0; i < 100; ++i) {
        PUSH_BACK_P(&vec, i, goto fault)
    }

    for (size_t i = 0; i < CNT(vec); ++i) {
        EXPECT_EQ((int) i, ID(vec, i));
    }

    EXPECT_LE(CNT(vec), SIZE(vec));

    FREE_V(&vec);

    EXPECT_EQ(vec, nullptr);

    return;

    fault:
    EXPECT_TRUE(false);
}

DEF_ARRAY(IntA, int)

void MakeZero(int *pInt) {
    *pInt = 0;
}

TEST(Array, Sanity) {
    IntA *intA;
    INIT_ARR(intA, 5, FAIL());

    ASSERT_EQ(SIZE(intA), 5U);

    for (size_t i = 0; i < SIZE(intA); ++i) {
        ID(intA, i) = static_cast<int>(i);
        EXPECT_EQ(ID(intA, i), static_cast<int>(i));
    }

    IntA *ref;
    CPYREF(intA, ref);

    FREE_A(intA, MakeZero);
    EXPECT_EQ(intA, nullptr);

    for (size_t i = 0; i < SIZE(ref); ++i) {
        EXPECT_EQ(ID(ref, i), static_cast<int>(i));
    }

    FREE_A(ref, MakeZero);
    EXPECT_EQ(ref, nullptr);
}
