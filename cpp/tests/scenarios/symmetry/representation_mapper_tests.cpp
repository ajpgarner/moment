/**
 * remapper_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/symmetrized/representation_mapper.h"

#include "../sparse_utils.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/locality/locality_context.h"

using namespace Moment::Symmetrized;

namespace Moment::Tests {
    TEST(Scenarios_Symmetry_RepresentationMapper, TwoOps_Id) {
        Algebraic::AlgebraicContext ac{2}; // two operators

        RepresentationMapper rm1{ac};
        EXPECT_EQ(rm1.target_word_length, 1);
        EXPECT_EQ(&rm1.context, &ac);
        ASSERT_EQ(rm1.raw_dimension(), 3); // 1, a, b
        ASSERT_EQ(rm1.remapped_dimension(), 3); // 1, a, b
        EXPECT_EQ(rm1[0], 0);
        EXPECT_EQ(rm1[1], 1);
        EXPECT_EQ(rm1[2], 2);
    }


    TEST(Scenarios_Symmetry_RepresentationMapper, TwoOps_1to2) {
        Algebraic::AlgebraicContext ac{2}; // two operators

        RepresentationMapper rm1{ac};
        RepresentationMapper remapper{ac, rm1, rm1, 2};

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

        const auto &lhs = remapper.LHS();
        EXPECT_EQ(lhs.nonZeros(), 9);
        EXPECT_EQ(lhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(lhs.coeff(1, 1), 1.0); // a
        EXPECT_EQ(lhs.coeff(2, 2), 1.0); // b
        EXPECT_EQ(lhs.coeff(1, 3), 1.0); // a alias
        EXPECT_EQ(lhs.coeff(3, 4), 1.0); // a^2
        EXPECT_EQ(lhs.coeff(4, 5), 1.0); // ab
        EXPECT_EQ(lhs.coeff(2, 6), 1.0); // b alias
        EXPECT_EQ(lhs.coeff(5, 7), 1.0); // ba
        EXPECT_EQ(lhs.coeff(6, 8), 1.0); // b^2

        const auto &rhs = remapper.RHS();
        EXPECT_EQ(rhs.nonZeros(), 7);
        EXPECT_EQ(rhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(rhs.coeff(1, 1), 1.0); // a
        EXPECT_EQ(rhs.coeff(2, 2), 1.0); // b
        EXPECT_EQ(rhs.coeff(4, 3), 1.0); // a^2 ; skip a
        EXPECT_EQ(rhs.coeff(5, 4), 1.0); // ab
        EXPECT_EQ(rhs.coeff(7, 5), 1.0); // ba ; skip b
        EXPECT_EQ(rhs.coeff(8, 6), 1.0); // b^2

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                const auto elem = one_elem(3, i, j);
                const auto mapped_elem = remapper(elem);
                ASSERT_EQ(mapped_elem.rows(), 7);
                ASSERT_EQ(mapped_elem.cols(), 7);
                EXPECT_EQ(mapped_elem.nonZeros(), 1);
            }
        }

        // Example symmetry group tx:, a,b->x implies aa, ab, ba, bb -> "x^2", hermitian
        const auto z2_rep1_av = make_sparse(3, {1, 0, 0,
                                                0, 0.5, 0.5,
                                                0, 0.5, 0.5});
        const auto expected_expand = make_sparse(7, {1, 0, 0, 0, 0, 0, 0,
                                                     0, 0.5, 0.5, 0, 0, 0, 0,
                                                     0, 0.5, 0.5, 0, 0, 0, 0,
                                                     0, 0, 0, 0.25, 0.25, 0.25, 0.25,
                                                     0, 0, 0, 0.25, 0.25, 0.25, 0.25,
                                                     0, 0, 0, 0.25, 0.25, 0.25, 0.25,
                                                     0, 0, 0, 0.25, 0.25, 0.25, 0.25});
        auto actual_expand = remapper(z2_rep1_av);
        EXPECT_TRUE(actual_expand.isApprox(expected_expand)) << actual_expand;
    }

    TEST(Scenarios_Symmetry_RepresentationMapper, Two_Ops_1234) {
        Algebraic::AlgebraicContext ac{2}; // two operators

        RepresentationMapper rm1{ac};
        RepresentationMapper rm2{ac, rm1, rm1, 2};
        RepresentationMapper rm3{ac, rm2, rm1, 3};
        RepresentationMapper rm4{ac, rm2, rm2, 4};

        EXPECT_EQ(rm1.raw_dimension(), 3);
        EXPECT_EQ(rm1.remapped_dimension(), 3);
        EXPECT_EQ(rm2.raw_dimension(), 9);
        EXPECT_EQ(rm2.remapped_dimension(), 7); // redundant ea -> a, redundant eb -> b
        EXPECT_EQ(rm3.raw_dimension(), 21);
        EXPECT_EQ(rm3.remapped_dimension(), 15);
        EXPECT_EQ(rm4.raw_dimension(), 49); // 7 * 7 -> 49; vs; 21 * 3 -> 63; could 'add one' be better?
        EXPECT_EQ(rm4.remapped_dimension(), 31);

    }


    TEST(Scenarios_Symmetry_RepresentationMapper, CHSH_1to2) {
        Locality::LocalityContext context{Locality::Party::MakeList(2, 2, 2)};

        RepresentationMapper rm1{context};
        ASSERT_EQ(rm1.raw_dimension(), 5);
        ASSERT_EQ(rm1.remapped_dimension(), 5);

        RepresentationMapper remapper{context, rm1, rm1, 2};
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

        // Check addition of values
        const auto &lhs = remapper.LHS();
        ASSERT_EQ(lhs.nonZeros(), 25);
        EXPECT_EQ(lhs.coeff(0, 0), 1.0);   // e
        EXPECT_EQ(lhs.coeff(1, 1), 1.0);   // a0
        EXPECT_EQ(lhs.coeff(2, 2), 1.0);   // a1
        EXPECT_EQ(lhs.coeff(3, 3), 1.0);   // b0
        EXPECT_EQ(lhs.coeff(4, 4), 1.0);   // b1
        EXPECT_EQ(lhs.coeff(1, 5), 1.0);   // a0 alias
        EXPECT_EQ(lhs.coeff(1, 6), 1.0);   // a0 alias
        EXPECT_EQ(lhs.coeff(5, 7), 1.0);   // a0a1
        EXPECT_EQ(lhs.coeff(6, 8), 1.0);   // a0b0
        EXPECT_EQ(lhs.coeff(7, 9), 1.0);   // a0b1
        EXPECT_EQ(lhs.coeff(2, 10), 1.0);  // a1 alias
        EXPECT_EQ(lhs.coeff(8, 11), 1.0);  // a1a0
        EXPECT_EQ(lhs.coeff(2, 12), 1.0);  // a1 alias
        EXPECT_EQ(lhs.coeff(9, 13), 1.0);  // a1b0
        EXPECT_EQ(lhs.coeff(10, 14), 1.0); // a1b1
        EXPECT_EQ(lhs.coeff(3, 15), 1.0);  // b0 alias
        EXPECT_EQ(lhs.coeff(6, 16), 1.0);  // a0b0 alias
        EXPECT_EQ(lhs.coeff(9, 17), 1.0);  // a1b0 alias
        EXPECT_EQ(lhs.coeff(3, 18), 1.0);  // b0 alias
        EXPECT_EQ(lhs.coeff(11, 19), 1.0); // b0b1
        EXPECT_EQ(lhs.coeff(4, 20), 1.0);  // b1 alias
        EXPECT_EQ(lhs.coeff(7, 21), 1.0);  // a0b1 alias
        EXPECT_EQ(lhs.coeff(10, 22), 1.0); // a1b1 alias
        EXPECT_EQ(lhs.coeff(12, 23), 1.0); // b1b0
        EXPECT_EQ(lhs.coeff(4, 24), 1.0);  // b1 alias

        // Check elision of redundant rows
        const auto &rhs = remapper.RHS();
        ASSERT_EQ(rhs.nonZeros(), 13);
        EXPECT_EQ(rhs.coeff(0, 0), 1.0); // e
        EXPECT_EQ(rhs.coeff(1, 1), 1.0); // a0
        EXPECT_EQ(rhs.coeff(2, 2), 1.0); // a1
        EXPECT_EQ(rhs.coeff(3, 3), 1.0); // b0
        EXPECT_EQ(rhs.coeff(4, 4), 1.0); // b1
        EXPECT_EQ(rhs.coeff(7, 5), 1.0); // a0a1 ; skip e a0, a0 a0
        EXPECT_EQ(rhs.coeff(8, 6), 1.0); // a0b0
        EXPECT_EQ(rhs.coeff(9, 7), 1.0); // a0b1
        EXPECT_EQ(rhs.coeff(11, 8), 1.0); // a1a0 ; skip e a1
        EXPECT_EQ(rhs.coeff(13, 9), 1.0); // a1b0 ; skip a1 a1
        EXPECT_EQ(rhs.coeff(14, 10), 1.0); // a1b1
        EXPECT_EQ(rhs.coeff(19, 11), 1.0); // b0b1 ; skip e b0, b0 a0, b0 a1, b0 b0
        EXPECT_EQ(rhs.coeff(23, 12), 1.0); // b1b0 ; skip e b1, b1 a0, b1 a1


        // Check "inversion of operators" symmetry:
        auto rep_base = make_sparse(5, {1, 1, 1, 1, 1,
                                        0, -1, 0, 0, 0,
                                        0, 0, -1, 0, 0,
                                        0, 0, 0, -1, 0,
                                        0, 0, 0, 0, -1});

        Eigen::SparseMatrix<double> expected_level2(13, 13);
        std::vector<Eigen::Triplet<double>> trips;
        trips.emplace_back(0, 0, 1.0); // e -> e
        trips.emplace_back(0, 1, 1.0); // a0 -> 1 - a0
        trips.emplace_back(1, 1, -1.0);
        trips.emplace_back(0, 2, 1.0); // a1 -> 1 - a1
        trips.emplace_back(2, 2, -1.0);
        trips.emplace_back(0, 3, 1.0); // b0 -> 1 - b0
        trips.emplace_back(3, 3, -1.0);
        trips.emplace_back(0, 4, 1.0); // b1 -> 1 - b1
        trips.emplace_back(4, 4, -1.0);
        trips.emplace_back(0, 5, 1.0); // a0a1 -> 1 - a0 - a1 + a0a1
        trips.emplace_back(1, 5, -1.0);
        trips.emplace_back(2, 5, -1.0);
        trips.emplace_back(5, 5, 1.0);
        trips.emplace_back(0, 6, 1.0); // a0b0 -> 1 - a0 - b0 + a0b0
        trips.emplace_back(1, 6, -1.0);
        trips.emplace_back(3, 6, -1.0);
        trips.emplace_back(6, 6, 1.0);
        trips.emplace_back(0, 7, 1.0); // a0b1 -> 1 - a0 - b1 + a0b1
        trips.emplace_back(1, 7, -1.0);
        trips.emplace_back(4, 7, -1.0);
        trips.emplace_back(7, 7, 1.0);
        trips.emplace_back(0, 8, 1.0); // a1a0 -> 1 - a0 - a1 + a1a0
        trips.emplace_back(1, 8, -1.0);
        trips.emplace_back(2, 8, -1.0);
        trips.emplace_back(8, 8, 1.0);
        trips.emplace_back(0, 9, 1.0); // a1b0 -> 1 - a1 - b0 + a1b0
        trips.emplace_back(2, 9, -1.0);
        trips.emplace_back(3, 9, -1.0);
        trips.emplace_back(9, 9, 1.0);
        trips.emplace_back(0, 10, 1.0); // a1b1 -> 1 - a1 - b1 + a1b1
        trips.emplace_back(2, 10, -1.0);
        trips.emplace_back(4, 10, -1.0);
        trips.emplace_back(10, 10, 1.0);
        trips.emplace_back(0, 11, 1.0); // b0b1 -> 1 - b0 - b1 + b0b1
        trips.emplace_back(3, 11, -1.0);
        trips.emplace_back(4, 11, -1.0);
        trips.emplace_back(11, 11, 1.0);
        trips.emplace_back(0, 12, 1.0); // b1b0 -> 1 - b0 - b1 + b1b0
        trips.emplace_back(3, 12, -1.0);
        trips.emplace_back(4, 12, -1.0);
        trips.emplace_back(12, 12, 1.0);
        expected_level2.setFromSortedTriplets(trips.begin(), trips.end());

        auto rep_level2 = remapper(rep_base);
        ASSERT_EQ(rep_level2.nonZeros(), expected_level2.nonZeros()) << rep_level2;
        EXPECT_TRUE(rep_level2.isApprox(expected_level2));
    }
}