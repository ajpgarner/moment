/**
 * remapper_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symmetry/remapper.h"

#include "sparse_utils.h"

#include "scenarios/algebraic/algebraic_context.h"

namespace Moment::Tests {

    TEST(Symmetry_Remapper, Remap1to2) {
        Algebraic::AlgebraicContext ac{2}; // two operators

        Remapper remapper{ac, 2};
        ASSERT_EQ(remapper.raw_dimension(), 9);
        ASSERT_EQ(remapper.remapped_dimension(), 7); // redundant ea -> a, redundant eb -> b

        EXPECT_EQ(remapper[0], 0); // e -> e
        EXPECT_EQ(remapper[1], 1); // a -> a
        EXPECT_EQ(remapper[2], 2); // b -> b
        EXPECT_EQ(remapper[3], 1); // a(e) -> a
        EXPECT_EQ(remapper[4], 3); // aa -> aa
        EXPECT_EQ(remapper[5], 4); // ab -> ab
        EXPECT_EQ(remapper[6], 2); // b(e) -> b
        EXPECT_EQ(remapper[7], 5); // ba -> ba
        EXPECT_EQ(remapper[8], 6); // bb -> bb

        const auto& lhs = remapper.LHS();
        EXPECT_EQ(lhs.nonZeros(), 7);
        EXPECT_EQ(lhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(lhs.coeff(1, 1), 1.0); // a
        EXPECT_EQ(lhs.coeff(2, 2), 1.0); // b
        EXPECT_EQ(lhs.coeff(3, 4), 1.0); // a^2 ; skip a
        EXPECT_EQ(lhs.coeff(4, 5), 1.0); // ab
        EXPECT_EQ(lhs.coeff(5, 7), 1.0); // ba ; skip b
        EXPECT_EQ(lhs.coeff(6, 8), 1.0); // b^2

        const auto& rhs = remapper.RHS();
        EXPECT_EQ(rhs.nonZeros(), 9);
        EXPECT_EQ(rhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(rhs.coeff(1, 1), 1.0); // a
        EXPECT_EQ(rhs.coeff(2, 2), 1.0); // b
        EXPECT_EQ(rhs.coeff(3, 1), 1.0); // a alias
        EXPECT_EQ(rhs.coeff(4, 3), 1.0); // a^2
        EXPECT_EQ(rhs.coeff(5, 4), 1.0); // ab
        EXPECT_EQ(rhs.coeff(6, 2), 1.0); // b alias
        EXPECT_EQ(rhs.coeff(7, 5), 1.0); // ba
        EXPECT_EQ(rhs.coeff(8, 6), 1.0); // b^2


        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                const auto elem = one_elem(3, i, j);
                const auto mapped_elem = remapper(elem);
                ASSERT_EQ(mapped_elem.rows(), 7);
                ASSERT_EQ(mapped_elem.cols(), 7);
                EXPECT_EQ(mapped_elem.nonZeros(), 1);
                //std::cout << "i = " << i << "j = " << j << " :\n" << mapped_elem << "\n";
            }
        }
    }
}