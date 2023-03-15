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
#include "scenarios/locality/locality_context.h"

namespace Moment::Tests {

    TEST(Symmetry_Remapper, Remap1to2_TwoOps) {
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

    TEST(Symmetry_Remapper, Remap1to2_CHSH) {
        Locality::LocalityContext context{Locality::Party::MakeList(2, 2, 2)};

        Remapper remapper{context, 2};
        ASSERT_EQ(remapper.raw_dimension(), 25);
        ASSERT_EQ(remapper.remapped_dimension(), 13);

        // Remap 25->13 comes from following operators:
        std::vector<size_t> expected_map = {0, 1, 2, 3, 4,    // e, a0, a1, b0, b1
                                            1, 1, 5, 6, 7,    // [a0], [a0], a0a1, a0b0, a0b1,
                                            2, 8, 2, 9, 10,   // [a1], a1a0, [a1], a1b0, a1b1,
                                            3, 6, 9, 3, 11,   // [b0], [a0b0], [a1b0], [b0], b0b1,
                                            4, 7, 10, 12, 4}; // [b1], [a0b1], [a1b1], b1b0, [b1]

        for (auto x = 0; x < 25; ++x) {
            ASSERT_EQ(remapper[x], expected_map[x]) << "Index " << x;
        }


        // Check elision of redundant rows
        const auto& lhs = remapper.LHS();
        ASSERT_EQ(lhs.nonZeros(), 13);
        EXPECT_EQ(lhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(lhs.coeff(1, 1), 1.0); // a0
        EXPECT_EQ(lhs.coeff(2, 2), 1.0); // a1
        EXPECT_EQ(lhs.coeff(3, 3), 1.0); // b0
        EXPECT_EQ(lhs.coeff(4, 4), 1.0); // b1
        EXPECT_EQ(lhs.coeff(5, 7), 1.0); // a0a1 ; skip e a0, a0 a0
        EXPECT_EQ(lhs.coeff(6, 8), 1.0); // a0b0
        EXPECT_EQ(lhs.coeff(7, 9), 1.0); // a0b1
        EXPECT_EQ(lhs.coeff(8, 11), 1.0); // a1a0 ; skip e a1
        EXPECT_EQ(lhs.coeff(9, 13), 1.0); // a1b0 ; skip a1 a1
        EXPECT_EQ(lhs.coeff(10, 14), 1.0); // a1b1
        EXPECT_EQ(lhs.coeff(11, 19), 1.0); // b0b1 ; skip e b0, b0 a0, b0 a1, b0 b0
        EXPECT_EQ(lhs.coeff(12, 23), 1.0); // b1b0 ; skip e b1, b1 a0, b1 a1

        // Check addition of values
        const auto& rhs = remapper.RHS();
        ASSERT_EQ(rhs.nonZeros(), 25);
        EXPECT_EQ(rhs.coeff(0, 0), 1.0);   // e
        EXPECT_EQ(rhs.coeff(1, 1), 1.0);   // a0
        EXPECT_EQ(rhs.coeff(2, 2), 1.0);   // a1
        EXPECT_EQ(rhs.coeff(3, 3), 1.0);   // b0
        EXPECT_EQ(rhs.coeff(4, 4), 1.0);   // b1
        EXPECT_EQ(rhs.coeff(5, 1), 1.0);   // a0 alias
        EXPECT_EQ(rhs.coeff(6, 1), 1.0);   // a0 alias
        EXPECT_EQ(rhs.coeff(7, 5), 1.0);   // a0a1
        EXPECT_EQ(rhs.coeff(8, 6), 1.0);   // a0b0
        EXPECT_EQ(rhs.coeff(9, 7), 1.0);   // a0b1
        EXPECT_EQ(rhs.coeff(10, 2), 1.0);  // a1 alias
        EXPECT_EQ(rhs.coeff(11, 8), 1.0);  // a1a0
        EXPECT_EQ(rhs.coeff(12, 2), 1.0);  // a1 alias
        EXPECT_EQ(rhs.coeff(13, 9), 1.0);  // a1b0
        EXPECT_EQ(rhs.coeff(14, 10), 1.0); // a1b1
        EXPECT_EQ(rhs.coeff(15, 3), 1.0);  // b0 alias
        EXPECT_EQ(rhs.coeff(16, 6), 1.0);  // a0b0 alias
        EXPECT_EQ(rhs.coeff(17, 9), 1.0);  // a1b0 alias
        EXPECT_EQ(rhs.coeff(18, 3), 1.0);  // b0 alias
        EXPECT_EQ(rhs.coeff(19, 11), 1.0); // b0b1
        EXPECT_EQ(rhs.coeff(20, 4), 1.0);  // b1 alias
        EXPECT_EQ(rhs.coeff(21, 7), 1.0);  // a0b1 alias
        EXPECT_EQ(rhs.coeff(22, 10), 1.0); // a1b1 alias
        EXPECT_EQ(rhs.coeff(23, 12), 1.0); // b1b0
        EXPECT_EQ(rhs.coeff(24, 4), 1.0);  // b1 alias

        // Check "inversion of operators" symmetry:
        auto rep_base = make_sparse(5, {1, 0, 0, 0, 0,
                                        1, -1, 0, 0, 0,
                                        1, 0, -1, 0, 0,
                                        1, 0, 0, -1, 0,
                                        1, 0, 0, 0, -1});

        Eigen::SparseMatrix<double> expected_level2(13, 13);
        std::vector<Eigen::Triplet<double>> trips;
        trips.emplace_back(0, 0, 1.0); // e -> e
        trips.emplace_back(1, 0, 1.0); // a0 -> 1 - a0
        trips.emplace_back(1, 1, -1.0);
        trips.emplace_back(2, 0, 1.0); // a1 -> 1 - a1
        trips.emplace_back(2, 2, -1.0);
        trips.emplace_back(3, 0, 1.0); // b0 -> 1 - b0
        trips.emplace_back(3, 3, -1.0);
        trips.emplace_back(4, 0, 1.0); // b1 -> 1 - b1
        trips.emplace_back(4, 4, -1.0);
        trips.emplace_back(5, 0, 1.0); // a0a1 -> 1 - a0 - a1 + a0a1
        trips.emplace_back(5, 1, -1.0);
        trips.emplace_back(5, 2, -1.0);
        trips.emplace_back(5, 5, 1.0);
        trips.emplace_back(6, 0, 1.0); // a0b0 -> 1 - a0 - b0 + a0b0
        trips.emplace_back(6, 1, -1.0);
        trips.emplace_back(6, 3, -1.0);
        trips.emplace_back(6, 6, 1.0);
        trips.emplace_back(7, 0, 1.0); // a0b1 -> 1 - a0 - b1 + a0b1
        trips.emplace_back(7, 1, -1.0);
        trips.emplace_back(7, 4, -1.0);
        trips.emplace_back(7, 7, 1.0);
        trips.emplace_back(8, 0, 1.0); // a1a0 -> 1 - a0 - a1 + a1a0
        trips.emplace_back(8, 1, -1.0);
        trips.emplace_back(8, 2, -1.0);
        trips.emplace_back(8, 8, 1.0);
        trips.emplace_back(9, 0, 1.0); // a1b0 -> 1 - a1 - b0 + a1b0
        trips.emplace_back(9, 2, -1.0);
        trips.emplace_back(9, 3, -1.0);
        trips.emplace_back(9, 9, 1.0);
        trips.emplace_back(10, 0, 1.0); // a1b1 -> 1 - a1 - b1 + a1b1
        trips.emplace_back(10, 2, -1.0);
        trips.emplace_back(10, 4, -1.0);
        trips.emplace_back(10, 10, 1.0);
        trips.emplace_back(11, 0, 1.0); // b0b1 -> 1 - b0 - b1 + b0b1
        trips.emplace_back(11, 3, -1.0);
        trips.emplace_back(11, 4, -1.0);
        trips.emplace_back(11, 11, 1.0);
        trips.emplace_back(12, 0, 1.0); // b1b0 -> 1 - b0 - b1 + b1b0
        trips.emplace_back(12, 3, -1.0);
        trips.emplace_back(12, 4, -1.0);
        trips.emplace_back(12, 12, 1.0);
        expected_level2.setFromTriplets(trips.begin(), trips.end());

        auto rep_level2 = remapper(rep_base);
        ASSERT_EQ(rep_level2.nonZeros(), expected_level2.nonZeros());
        EXPECT_TRUE(rep_level2.isApprox(expected_level2));

    }
}