/**
 * ipow_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/ipow.h"

namespace NPATK::Tests {

    TEST(IntegerPower, Pow0_0) {
        EXPECT_EQ(ipow(0, 0), 1);
    }

    TEST(IntegerPower, Pow0_1) {
        EXPECT_EQ(ipow(0, 1), 0);
    }

    TEST(IntegerPower, Pow1_0) {
        EXPECT_EQ(ipow(1, 0), 1);
    }

    TEST(IntegerPower, Pow2_0) {
        EXPECT_EQ(ipow(2, 0), 1);
    }

    TEST(IntegerPower, Pow2_2) {
        EXPECT_EQ(ipow(2, 2), 4);
    }

    TEST(IntegerPower, Pow2_3) {
        EXPECT_EQ(ipow(2, 3), 8);
    }

    TEST(IntegerPower, Pow4_5) {
        EXPECT_EQ(ipow(4, 5), 1024);
    }

    TEST(IntegerPower, Pow13_0) {
        EXPECT_EQ(ipow(13, 0), 1);
    }

    TEST(IntegerPower, Pow13_1) {
        EXPECT_EQ(ipow(13, 1), 13);
    }

    TEST(IntegerPower, Pow13_2) {
        EXPECT_EQ(ipow(13, 2), 169);
    }
}