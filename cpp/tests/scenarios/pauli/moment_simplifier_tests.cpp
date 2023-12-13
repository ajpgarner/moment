/**
 * nonwrapping_simplifier_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/symmetry/moment_simplifier_no_wrapping.h"
#include "scenarios/pauli/symmetry/moment_simplifier_wrapping.h"

#include <array>
#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapChain_Empty) {
        PauliContext empty{0};
        MomentSimplifierNoWrappingChain simplifier{empty};

        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapLattice_Empty) {
        PauliContext empty{0};
        MomentSimplifierNoWrappingLattice simplifier{empty};
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapChain_Minimum) {
        PauliContext context{10, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingChain simplifier{context};
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);

        EXPECT_EQ(simplifier.chain_minimum(context.zero()), 0);
        EXPECT_EQ(simplifier.chain_minimum(context.identity()), 0);

        EXPECT_EQ(simplifier.qubits, 10);
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaX(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaY(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaZ(qubit)), qubit) << " qubit = " << qubit;
        }

        EXPECT_EQ(simplifier.chain_minimum(context.sigmaX(0) * context.sigmaY(5)), 0);
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapLattice_Minimum) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingLattice simplifier{context};
        EXPECT_EQ(simplifier.qubits, 16);
        EXPECT_EQ(simplifier.row_width, 4);
        EXPECT_EQ(simplifier.column_height, 4);
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);

        EXPECT_EQ(simplifier.lattice_minimum(context.zero()), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(simplifier.lattice_minimum(context.identity()),  (std::pair<size_t, size_t>{0, 0}));

        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(simplifier.lattice_minimum(context.sigmaX(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_minimum(context.sigmaY(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_minimum(context.sigmaZ(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
            }
        }
        EXPECT_EQ(simplifier.lattice_minimum(context.sigmaX(1,2) * context.sigmaY(0,3)),
                 (std::pair<size_t, size_t>{0, 2}));
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapChain_Maximum) {
        PauliContext context{10, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingChain simplifier{context};
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);

        EXPECT_EQ(simplifier.chain_maximum(context.zero()), 10);
        EXPECT_EQ(simplifier.chain_maximum(context.identity()), 10);

        EXPECT_EQ(simplifier.qubits, 10);
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_maximum(context.sigmaX(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_maximum(context.sigmaY(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_maximum(context.sigmaZ(qubit)), qubit) << " qubit = " << qubit;
        }

        EXPECT_EQ(simplifier.chain_maximum(context.sigmaX(0) * context.sigmaY(5)), 5);
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapLattice_Maximum) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingLattice simplifier{context};
        EXPECT_EQ(simplifier.qubits, 16);
        EXPECT_EQ(simplifier.row_width, 4);
        EXPECT_EQ(simplifier.column_height, 4);
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);

        EXPECT_EQ(simplifier.lattice_maximum(context.zero()), (std::pair<size_t, size_t>{4, 4}));
        EXPECT_EQ(simplifier.lattice_maximum(context.identity()),  (std::pair<size_t, size_t>{4, 4}));

        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(simplifier.lattice_maximum(context.sigmaX(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_maximum(context.sigmaY(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_maximum(context.sigmaZ(row, col)),
                          (std::pair<size_t, size_t>{row, col})) << " row = " << row << ", col = " << col;
            }
        }
        EXPECT_EQ(simplifier.lattice_maximum(context.sigmaX(0, 3) * context.sigmaY(1, 2)),
                  (std::pair<size_t, size_t>{1, 3}));
    }


    TEST(Scenarios_Pauli_SiteHasher, WrappingChainSmall_CanonicalSequence) {
        const size_t chain_length = 5;
        PauliContext context{chain_length, WrapType::Wrap, SymmetryType::Translational};
        MomentSimplifierWrapping<1> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single{hasher.canonical_sequence(shifted_single_sequence)};

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn{hasher.canonical_sequence(shifted_nn_sequence)};
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, WrappingChainMedium_CanonicalSequence) {
        const size_t chain_length = 40;
        PauliContext context{chain_length, WrapType::Wrap, SymmetryType::Translational};
        MomentSimplifierWrapping<2> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single = hasher.canonical_sequence(shifted_single_sequence);

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn = hasher.canonical_sequence(shifted_nn_sequence);
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }

    TEST(Scenarios_Pauli_SiteHasher, WrappingChainLarge_CanonicalSequence) {
        const size_t chain_length = 72;
        PauliContext context{chain_length, WrapType::Wrap, SymmetryType::Translational};
        MomentSimplifierWrapping<3> hasher{context};

        // Canonical results:
        const auto expected_single = context.sigmaX(0);
        const auto expected_nn = context.sigmaX(0) * context.sigmaY(1);

        for (size_t base_index = 0; base_index < chain_length; ++base_index) {
            // Single qubit
            const auto shifted_single_sequence = context.sigmaX(base_index);
            const OperatorSequence canonical_single{hasher.canonical_sequence(shifted_single_sequence)};

            EXPECT_EQ(canonical_single, expected_single) << "site = " << base_index;

            // Nearest neighbour
            const auto shifted_nn_sequence =
                    context.sigmaX(base_index) * context.sigmaY((base_index + 1) % chain_length);
            const OperatorSequence canonical_nn{hasher.canonical_sequence(shifted_nn_sequence)};
            EXPECT_EQ(canonical_nn, expected_nn) << "site = " << base_index;
        }
    }
}