/**
 * lattice_duplicator_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "dictionary/raw_polynomial.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/symmetry/lattice_duplicator.h"

#include <array>
#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_LatticeDuplicator, OneQubitFill) {
        PauliContext lattice{4, 4, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        duplicator.one_qubit_fill(5);
        ASSERT_EQ(output_list.size(), 3);
        EXPECT_EQ(output_list[0], lattice.sigmaX(5));
        EXPECT_EQ(output_list[1], lattice.sigmaY(5));
        EXPECT_EQ(output_list[2], lattice.sigmaZ(5));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, TwoQubitFill_Ordered) {
        PauliContext lattice{4, 4, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        duplicator.two_qubit_fill(5, 10);
        ASSERT_EQ(output_list.size(), 9);
        EXPECT_EQ(output_list[0], lattice.sigmaX(5) * lattice.sigmaX(10));
        EXPECT_EQ(output_list[1], lattice.sigmaX(5) * lattice.sigmaY(10));
        EXPECT_EQ(output_list[2], lattice.sigmaX(5) * lattice.sigmaZ(10));
        EXPECT_EQ(output_list[3], lattice.sigmaY(5) * lattice.sigmaX(10));
        EXPECT_EQ(output_list[4], lattice.sigmaY(5) * lattice.sigmaY(10));
        EXPECT_EQ(output_list[5], lattice.sigmaY(5) * lattice.sigmaZ(10));
        EXPECT_EQ(output_list[6], lattice.sigmaZ(5) * lattice.sigmaX(10));
        EXPECT_EQ(output_list[7], lattice.sigmaZ(5) * lattice.sigmaY(10));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(5) * lattice.sigmaZ(10));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, TwoQubitFill_OutOfOrder) {
        PauliContext lattice{4, 4, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        duplicator.two_qubit_fill(12, 2);
        ASSERT_EQ(output_list.size(), 9);
        EXPECT_EQ(output_list[0], lattice.sigmaX(2) * lattice.sigmaX(12));
        EXPECT_EQ(output_list[1], lattice.sigmaY(2) * lattice.sigmaX(12));
        EXPECT_EQ(output_list[2], lattice.sigmaZ(2) * lattice.sigmaX(12));
        EXPECT_EQ(output_list[3], lattice.sigmaX(2) * lattice.sigmaY(12));
        EXPECT_EQ(output_list[4], lattice.sigmaY(2) * lattice.sigmaY(12));
        EXPECT_EQ(output_list[5], lattice.sigmaZ(2) * lattice.sigmaY(12));
        EXPECT_EQ(output_list[6], lattice.sigmaX(2) * lattice.sigmaZ(12));
        EXPECT_EQ(output_list[7], lattice.sigmaY(2) * lattice.sigmaZ(12));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(2) * lattice.sigmaZ(12));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, ThreeQubitFill) {
        PauliContext lattice{4, 4, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        std::array<size_t, 3> sites{1, 5, 9};
        auto [first_idx, last_idx] = duplicator.permutation_fill(sites);
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 27);
        ASSERT_EQ(output_list.size(), 27);

        EXPECT_EQ(output_list[0],  lattice.sigmaX(1) * lattice.sigmaX(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[1],  lattice.sigmaX(1) * lattice.sigmaX(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[2],  lattice.sigmaX(1) * lattice.sigmaX(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[3],  lattice.sigmaX(1) * lattice.sigmaY(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[4],  lattice.sigmaX(1) * lattice.sigmaY(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[5],  lattice.sigmaX(1) * lattice.sigmaY(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[6],  lattice.sigmaX(1) * lattice.sigmaZ(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[7],  lattice.sigmaX(1) * lattice.sigmaZ(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[8],  lattice.sigmaX(1) * lattice.sigmaZ(5) * lattice.sigmaZ(9));

        EXPECT_EQ(output_list[9],  lattice.sigmaY(1) * lattice.sigmaX(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[10], lattice.sigmaY(1) * lattice.sigmaX(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[11], lattice.sigmaY(1) * lattice.sigmaX(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[12], lattice.sigmaY(1) * lattice.sigmaY(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[13], lattice.sigmaY(1) * lattice.sigmaY(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[14], lattice.sigmaY(1) * lattice.sigmaY(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[15], lattice.sigmaY(1) * lattice.sigmaZ(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[16], lattice.sigmaY(1) * lattice.sigmaZ(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[17], lattice.sigmaY(1) * lattice.sigmaZ(5) * lattice.sigmaZ(9));

        EXPECT_EQ(output_list[18], lattice.sigmaZ(1) * lattice.sigmaX(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[19], lattice.sigmaZ(1) * lattice.sigmaX(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[20], lattice.sigmaZ(1) * lattice.sigmaX(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[21], lattice.sigmaZ(1) * lattice.sigmaY(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[22], lattice.sigmaZ(1) * lattice.sigmaY(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[23], lattice.sigmaZ(1) * lattice.sigmaY(5) * lattice.sigmaZ(9));
        EXPECT_EQ(output_list[24], lattice.sigmaZ(1) * lattice.sigmaZ(5) * lattice.sigmaX(9));
        EXPECT_EQ(output_list[25], lattice.sigmaZ(1) * lattice.sigmaZ(5) * lattice.sigmaY(9));
        EXPECT_EQ(output_list[26], lattice.sigmaZ(1) * lattice.sigmaZ(5) * lattice.sigmaZ(9));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateChainOne) {
        PauliContext lattice{5, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0});
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 15);
        ASSERT_EQ(output_list.size(), 15);
        EXPECT_EQ(output_list[0], lattice.sigmaX(0));
        EXPECT_EQ(output_list[1], lattice.sigmaY(0));
        EXPECT_EQ(output_list[2], lattice.sigmaZ(0));
        EXPECT_EQ(output_list[3], lattice.sigmaX(1));
        EXPECT_EQ(output_list[4], lattice.sigmaY(1));
        EXPECT_EQ(output_list[5], lattice.sigmaZ(1));
        EXPECT_EQ(output_list[6], lattice.sigmaX(2));
        EXPECT_EQ(output_list[7], lattice.sigmaY(2));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(2));
        EXPECT_EQ(output_list[9], lattice.sigmaX(3));
        EXPECT_EQ(output_list[10], lattice.sigmaY(3));
        EXPECT_EQ(output_list[11], lattice.sigmaZ(3));
        EXPECT_EQ(output_list[12], lattice.sigmaX(4));
        EXPECT_EQ(output_list[13], lattice.sigmaY(4));
        EXPECT_EQ(output_list[14], lattice.sigmaZ(4));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateChainTwo) {
        PauliContext lattice{5, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0, 1});
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 45);
        ASSERT_EQ(output_list.size(), 45);

        for (size_t base_idx = 0; base_idx < 5; ++base_idx) {
            EXPECT_EQ(output_list[9*base_idx],     lattice.sigmaX(base_idx) * lattice.sigmaX((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 1], lattice.sigmaX(base_idx) * lattice.sigmaY((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 2], lattice.sigmaX(base_idx) * lattice.sigmaZ((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 3], lattice.sigmaY(base_idx) * lattice.sigmaX((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 4], lattice.sigmaY(base_idx) * lattice.sigmaY((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 5], lattice.sigmaY(base_idx) * lattice.sigmaZ((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 6], lattice.sigmaZ(base_idx) * lattice.sigmaX((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 7], lattice.sigmaZ(base_idx) * lattice.sigmaY((base_idx+1)%5));
            EXPECT_EQ(output_list[9*base_idx + 8], lattice.sigmaZ(base_idx) * lattice.sigmaZ((base_idx+1)%5));
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateAliasedChain) {
        PauliContext lattice{6, WrapType::Wrap, SymmetryType::Translational};


        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());
        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0, 3}, true);
        EXPECT_EQ(output_list.size(), 27);
        for (size_t base_idx = 0; base_idx < 3; ++base_idx) {
            EXPECT_EQ(output_list[9*base_idx],     lattice.sigmaX(base_idx) * lattice.sigmaX((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 1], lattice.sigmaX(base_idx) * lattice.sigmaY((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 2], lattice.sigmaX(base_idx) * lattice.sigmaZ((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 3], lattice.sigmaY(base_idx) * lattice.sigmaX((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 4], lattice.sigmaY(base_idx) * lattice.sigmaY((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 5], lattice.sigmaY(base_idx) * lattice.sigmaZ((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 6], lattice.sigmaZ(base_idx) * lattice.sigmaX((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 7], lattice.sigmaZ(base_idx) * lattice.sigmaY((base_idx+3)));
            EXPECT_EQ(output_list[9*base_idx + 8], lattice.sigmaZ(base_idx) * lattice.sigmaZ((base_idx+3)));
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateLatticeOne) {
        PauliContext lattice{2, 2, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0});
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 12);
        ASSERT_EQ(output_list.size(), 12);
        EXPECT_EQ(output_list[0], lattice.sigmaX(0));
        EXPECT_EQ(output_list[1], lattice.sigmaY(0));
        EXPECT_EQ(output_list[2], lattice.sigmaZ(0));
        EXPECT_EQ(output_list[3], lattice.sigmaX(1));
        EXPECT_EQ(output_list[4], lattice.sigmaY(1));
        EXPECT_EQ(output_list[5], lattice.sigmaZ(1));
        EXPECT_EQ(output_list[6], lattice.sigmaX(2));
        EXPECT_EQ(output_list[7], lattice.sigmaY(2));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(2));
        EXPECT_EQ(output_list[9], lattice.sigmaX(3));
        EXPECT_EQ(output_list[10], lattice.sigmaY(3));
        EXPECT_EQ(output_list[11], lattice.sigmaZ(3));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateLattice_22Checkerboard) {
        PauliContext lattice{2, 2, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0, 2}, true);
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 18);
        ASSERT_EQ(output_list.size(), 18);

        EXPECT_EQ(output_list[0], lattice.sigmaX(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[1], lattice.sigmaX(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[2], lattice.sigmaX(0) * lattice.sigmaZ(2));
        EXPECT_EQ(output_list[3], lattice.sigmaY(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[4], lattice.sigmaY(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[5], lattice.sigmaY(0) * lattice.sigmaZ(2));
        EXPECT_EQ(output_list[6], lattice.sigmaZ(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[7], lattice.sigmaZ(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(0) * lattice.sigmaZ(2));
        
        EXPECT_EQ(output_list[9], lattice.sigmaX(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[10], lattice.sigmaX(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[11], lattice.sigmaX(1) * lattice.sigmaZ(3));
        EXPECT_EQ(output_list[12], lattice.sigmaY(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[13], lattice.sigmaY(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[14], lattice.sigmaY(1) * lattice.sigmaZ(3));
        EXPECT_EQ(output_list[15], lattice.sigmaZ(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[16], lattice.sigmaZ(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[17], lattice.sigmaZ(1) * lattice.sigmaZ(3));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateLattice_33HorzLine) {
        PauliContext lattice{3, 3, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0, 3, 6}, true);
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 81);
        ASSERT_EQ(output_list.size(), 81);

        EXPECT_EQ(output_list[0],  lattice.sigmaX(0) * lattice.sigmaX(3) * lattice.sigmaX(6));
        EXPECT_EQ(output_list[27], lattice.sigmaX(1) * lattice.sigmaX(4) * lattice.sigmaX(7));
        EXPECT_EQ(output_list[54], lattice.sigmaX(2) * lattice.sigmaX(5) * lattice.sigmaX(8));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, DuplicateLattice_33VertLine) {
        PauliContext lattice{3, 3, WrapType::Wrap, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.symmetrical_fill(std::vector<size_t>{0, 1, 2}, true);
        EXPECT_EQ(first_idx, 0);
        EXPECT_EQ(last_idx, 81);
        ASSERT_EQ(output_list.size(), 81);

        EXPECT_EQ(output_list[0],  lattice.sigmaX(0) * lattice.sigmaX(1) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[27], lattice.sigmaX(3) * lattice.sigmaX(4) * lattice.sigmaX(5));
        EXPECT_EQ(output_list[54], lattice.sigmaX(6) * lattice.sigmaX(7) * lattice.sigmaX(8));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, WraplessFillLattice_OneQubit) {
        PauliContext lattice{2, 2, WrapType::None, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.wrapless_symmetrical_fill(std::vector<size_t>{0});
        EXPECT_EQ(first_idx, 0);
        ASSERT_EQ(last_idx, 12);
        ASSERT_EQ(output_list.size(), 12);
        EXPECT_EQ(output_list[0], lattice.sigmaX(0));
        EXPECT_EQ(output_list[1], lattice.sigmaY(0));
        EXPECT_EQ(output_list[2], lattice.sigmaZ(0));
        EXPECT_EQ(output_list[3], lattice.sigmaX(1));
        EXPECT_EQ(output_list[4], lattice.sigmaY(1));
        EXPECT_EQ(output_list[5], lattice.sigmaZ(1));
        EXPECT_EQ(output_list[6], lattice.sigmaX(2));
        EXPECT_EQ(output_list[7], lattice.sigmaY(2));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(2));
        EXPECT_EQ(output_list[9], lattice.sigmaX(3));
        EXPECT_EQ(output_list[10], lattice.sigmaY(3));
        EXPECT_EQ(output_list[11], lattice.sigmaZ(3));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, WraplessFillLattice_Pair) {
        PauliContext lattice{2, 2, WrapType::None, SymmetryType::Translational};
        std::vector<OperatorSequence> output_list;
        LatticeDuplicator duplicator{lattice, output_list};
        ASSERT_TRUE(output_list.empty());

        auto [first_idx, last_idx] = duplicator.wrapless_symmetrical_fill(std::array<size_t, 2>{0, 2});
        EXPECT_EQ(first_idx, 0);
        ASSERT_EQ(last_idx, 18);
        ASSERT_EQ(output_list.size(), 18);
        
        EXPECT_EQ(output_list[0], lattice.sigmaX(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[1], lattice.sigmaX(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[2], lattice.sigmaX(0) * lattice.sigmaZ(2));
        EXPECT_EQ(output_list[3], lattice.sigmaY(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[4], lattice.sigmaY(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[5], lattice.sigmaY(0) * lattice.sigmaZ(2));
        EXPECT_EQ(output_list[6], lattice.sigmaZ(0) * lattice.sigmaX(2));
        EXPECT_EQ(output_list[7], lattice.sigmaZ(0) * lattice.sigmaY(2));
        EXPECT_EQ(output_list[8], lattice.sigmaZ(0) * lattice.sigmaZ(2));
        
        EXPECT_EQ(output_list[9], lattice.sigmaX(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[10], lattice.sigmaX(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[11], lattice.sigmaX(1) * lattice.sigmaZ(3));
        EXPECT_EQ(output_list[12], lattice.sigmaY(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[13], lattice.sigmaY(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[14], lattice.sigmaY(1) * lattice.sigmaZ(3));
        EXPECT_EQ(output_list[15], lattice.sigmaZ(1) * lattice.sigmaX(3));
        EXPECT_EQ(output_list[16], lattice.sigmaZ(1) * lattice.sigmaY(3));
        EXPECT_EQ(output_list[17], lattice.sigmaZ(1) * lattice.sigmaZ(3));

    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_NoWrapChain_Single) {
        PauliContext chain{5, WrapType::None, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(chain.sigmaX(0), 1.0);
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(chain, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 5);
        for (size_t qubit = 0; qubit < 5; ++qubit) {
            EXPECT_EQ(duplicated_poly[qubit].sequence, chain.sigmaX(qubit)) << "qubit = " << qubit;
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_NoWrapChain_Neighbour) {
        PauliContext chain{5, WrapType::None, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(chain.sigmaX(0) * chain.sigmaY(1), 1.0);
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(chain, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 4);
        for (size_t qubit = 0; qubit < 4; ++qubit) {
            EXPECT_EQ(duplicated_poly[qubit].sequence, chain.sigmaX(qubit) * chain.sigmaY(qubit+1))
                    << "qubit = " << qubit;
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_WrappingChain_Single) {
        PauliContext chain{5, WrapType::Wrap, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(chain.sigmaX(0), 1.0);
        ASSERT_FALSE(base_poly.is_scalar());
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(chain, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 5);
        for (size_t qubit = 0; qubit < 5; ++qubit) {
            EXPECT_EQ(duplicated_poly[qubit].sequence, chain.sigmaX(qubit)) << "qubit = " << qubit;
        }
    }


    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_WrappingChain_Neighbour) {
        PauliContext chain{5, WrapType::Wrap, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(chain.sigmaX(0) * chain.sigmaY(1), 1.0);
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(chain, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 5);
        for (size_t qubit = 0; qubit < 4; ++qubit) {
            EXPECT_EQ(duplicated_poly[qubit].sequence, chain.sigmaX(qubit) * chain.sigmaY(qubit+1))
                                << "qubit = " << qubit;
        }
        EXPECT_EQ(duplicated_poly[4].sequence, chain.sigmaX(4) * chain.sigmaY(0));
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_NoWrapLattice_Single) {
        PauliContext lattice{3, 3, WrapType::None, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(lattice.sigmaX(0, 0), 1.0);
        ASSERT_FALSE(base_poly.is_scalar());
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(lattice, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 9);

        for (size_t col = 0; col < 3; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                EXPECT_EQ(duplicated_poly[col*3 + row].sequence, lattice.sigmaX(row, col))
                    << "row = " << row << ", col = " << col;
            }
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_NoWrapLattice_HorzNeighbour) {
        PauliContext lattice{3, 3, WrapType::None, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(lattice.sigmaX(0, 0) * lattice.sigmaY(0, 1), 1.0);
        ASSERT_FALSE(base_poly.is_scalar());
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(lattice, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 6);

        for (size_t col = 0; col < 2; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                EXPECT_EQ(duplicated_poly[col*3 + row].sequence, lattice.sigmaX(row, col) * lattice.sigmaY(row, col+1))
                    << "row = " << row << ", col = " << col;
            }
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_WrappingLattice_Single) {
        PauliContext lattice{3, 3, WrapType::Wrap, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(lattice.sigmaX(0, 0), 1.0);
        ASSERT_FALSE(base_poly.is_scalar());
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(lattice, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 9);

        for (size_t col = 0; col < 3; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                EXPECT_EQ(duplicated_poly[col*3 + row].sequence, lattice.sigmaX(row, col))
                    << "row = " << row << ", col = " << col;
            }
        }
    }

    TEST(Scenarios_Pauli_LatticeDuplicator, CopyRawPolynomial_WrappingLattice_HorzNeighbour) {
        PauliContext lattice{3, 3, WrapType::Wrap, SymmetryType::Translational};
        RawPolynomial base_poly;
        base_poly.emplace_back(lattice.sigmaX(0, 0) * lattice.sigmaY(0, 1), 1.0);
        ASSERT_FALSE(base_poly.is_scalar());
        const auto duplicated_poly = LatticeDuplicator::symmetrical_copy(lattice, base_poly);
        ASSERT_EQ(duplicated_poly.size(), 9);

        for (size_t col = 0; col < 3; ++col) {
            for (size_t row = 0; row < 3; ++row) {
                EXPECT_EQ(duplicated_poly[col*3 + row].sequence,
                          lattice.sigmaX(row, col) * lattice.sigmaY(row, (col+1)%3))
                                    << "row = " << row << ", col = " << col;
            }
        }
    }


}