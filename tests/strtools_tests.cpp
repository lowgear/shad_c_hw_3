#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

#include "gtest/gtest.h"

extern "C" {
#include "utils/strtools.h"
}

#define SIMPLE_CHECK(case, val) \
TEST(ParseInt32, case) { \
    int32_t res; \
    EXPECT_TRUE(ParseInt32(#val, nullptr)); \
    EXPECT_TRUE(ParseInt32(#val, &res)); \
    EXPECT_EQ(res, val); \
}

SIMPLE_CHECK(Sanity, 123)

SIMPLE_CHECK(OnNegative, -123)

SIMPLE_CHECK(OnSigned, +123)

SIMPLE_CHECK(Zero, 0)

SIMPLE_CHECK(NegZero, -0)

SIMPLE_CHECK(SignedZero, +0)

#undef SIMPLE_CHECK

TEST(ParseInt32, LeadingZeroes) {
    int32_t res;
    EXPECT_TRUE(ParseInt32("00123", nullptr));
    EXPECT_TRUE(ParseInt32("00123", &res));
    EXPECT_EQ(res, 123);
}

TEST(ParseInt32, CheckFalsePositives) {
#define CHECK(str) EXPECT_FALSE(ParseInt32(str, nullptr))
    CHECK("");
    CHECK("-");
    CHECK("+");
    CHECK("+-");
    CHECK("-+");
    CHECK("--1");
    CHECK("1.1");
    CHECK("1.1");
    CHECK("abracadabra");
    CHECK("123 123");
    CHECK(" 123");
    CHECK("123  ");
#undef CHECK
}

#pragma clang diagnostic pop