/**
 * lattice_duplicator_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/lattice_duplicator.h"

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
}