/**
 * kronecker_power_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "../scenarios/sparse_utils.h"

#include "utilities/kronecker_power.h"

namespace Moment::Tests {

    TEST(Utilities_KroneckerPower, PowM_0) {

        auto m = make_sparse(2, {2.0, 3.0, 5.0, 7.0});
        auto m0 = kronecker_power(m, 0);

        auto expected = sparse_id(1);

        EXPECT_TRUE(m0.isApprox(expected));
    }

    TEST(Utilities_KroneckerPower, PowM_1) {

        auto m = make_sparse(2, {2.0, 3.0, 5.0, 7.0});
        auto m1 = kronecker_power(m, 1);
        EXPECT_TRUE(m1.isApprox(m));
    }

    TEST(Utilities_KroneckerPower, PowM_2) {

        auto m = make_sparse(2, {2.0, 3.0, 5.0, 7.0});
        auto m2 = kronecker_power(m, 2);
        auto expected = make_sparse<double>(4, {4,  6,  6,  9,
                                                10, 14, 15, 21,
                                                10, 15, 14, 21,
                                                25, 35, 35, 49});
        EXPECT_TRUE(m2.isApprox(expected));
    }

    TEST(Utilities_KroneckerPower, PowM_3) {

        auto m = make_sparse(2, {2.0, 3.0, 5.0, 7.0});
        auto m3 = kronecker_power(m, 3);
        auto expected = make_sparse<double>(8, {8, 12, 12, 18, 12, 18, 18, 27,
                                                20, 28, 30, 42, 30, 42, 45, 63,
                                                20, 30, 28, 42, 30, 45, 42, 63,
                                                50, 70, 70, 98, 75, 105, 105, 147,
                                                20, 30, 30, 45, 28, 42, 42, 63,
                                                50, 70, 75, 105, 70, 98, 105, 147,
                                                50, 75, 70, 105, 70, 105, 98, 147,
                                                125, 175, 175, 245, 175, 245, 245, 343});
        EXPECT_TRUE(m3.isApprox(expected));
    }

    TEST(Utilities_KroneckerPower, PowM_4) {
        auto m = make_sparse(2, {2.0, 3.0, 5.0, 7.0});
        auto m4 = kronecker_power(m, 4);
        auto expected = make_sparse<double>(16,
            {16, 24, 24, 36, 24, 36, 36, 54, 24, 36, 36, 54, 36, 54, 54, 81,
             40, 56, 60, 84, 60, 84, 90, 126, 60, 84, 90, 126, 90, 126, 135, 189,
             40, 60, 56, 84, 60, 90, 84, 126, 60, 90, 84, 126, 90, 135, 126, 189,
             100, 140, 140, 196, 150, 210, 210, 294, 150, 210, 210, 294, 225, 315, 315, 441,
             40, 60, 60, 90, 56, 84, 84, 126, 60, 90, 90, 135, 84, 126, 126, 189,
             100, 140, 150, 210, 140, 196, 210, 294, 150, 210, 225, 315, 210, 294, 315, 441,
             100, 150, 140, 210, 140, 210, 196, 294, 150, 225, 210, 315, 210, 315, 294, 441,
             250, 350, 350, 490, 350, 490, 490, 686, 375, 525, 525, 735, 525, 735, 735, 1029,
             40, 60, 60, 90, 60, 90, 90, 135, 56, 84, 84, 126, 84, 126, 126, 189,
             100, 140, 150, 210, 150, 210, 225, 315, 140, 196, 210, 294, 210, 294, 315, 441,
             100, 150, 140, 210, 150, 225, 210, 315, 140, 210, 196, 294, 210, 315, 294, 441,
             250, 350, 350, 490, 375, 525, 525, 735, 350, 490, 490, 686, 525, 735, 735, 1029,
             100, 150, 150, 225, 140, 210, 210, 315, 140, 210, 210, 315, 196, 294, 294, 441,
             250, 350, 375, 525, 350, 490, 525, 735, 350, 490, 525, 735, 490, 686, 735, 1029,
             250, 375, 350, 525, 350, 525, 490, 735, 350, 525, 490, 735, 490, 735, 686, 1029,
             625, 875, 875, 1225, 875, 1225, 1225, 1715, 875, 1225, 1225, 1715, 1225, 1715, 1715, 2401});
        EXPECT_TRUE(m4.isApprox(expected));
    }
}