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

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapChain_Supremum) {
        PauliContext context{10, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingChain simplifier{context};
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);

        EXPECT_EQ(simplifier.chain_supremum(context.zero()), 0);
        EXPECT_EQ(simplifier.chain_supremum(context.identity()), 0);

        EXPECT_EQ(simplifier.qubits, 10);
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_supremum(context.sigmaX(qubit)), 1+qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_supremum(context.sigmaY(qubit)), 1+qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_supremum(context.sigmaZ(qubit)), 1+qubit) << " qubit = " << qubit;
        }

        EXPECT_EQ(simplifier.chain_supremum(context.sigmaX(0) * context.sigmaY(5)), 6);
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapChain_Offset) {
        PauliContext context{10, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingChain simplifier{context};
        EXPECT_EQ(simplifier.chain_supremum(context.identity()), 0);
        EXPECT_EQ(simplifier.qubits, 10);
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_offset(context.sigmaX(0), qubit),
                      context.sigmaX(qubit)) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_offset(context.sigmaY(0), qubit),
                      context.sigmaY(qubit)) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_offset(context.sigmaZ(0), qubit),
                      context.sigmaZ(qubit)) << " qubit = " << qubit;
        }

        EXPECT_EQ(simplifier.chain_offset(context.sigmaX(0) * context.sigmaY(3), 2),
                  context.sigmaX(2) * context.sigmaY(5));
    }


    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapLattice_Offset) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        const auto& simplifier = context.moment_simplifier();
        ASSERT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaX(0, 0), row, col),
                          context.sigmaX(row, col)) << " col = " << col << ", row = " << row;
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaY(0, 0), row, col),
                          context.sigmaY(row, col)) << " col = " << col << ", row = " << row;
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaZ(0, 0), row, col),
                          context.sigmaZ(row, col)) << " col = " << col << ", row = " << row;
            }
        }

        EXPECT_EQ(simplifier.lattice_offset(context.sigmaX(0, 0) * context.sigmaZ(0, 2), 0, 1),
                  context.sigmaX(0, 1) * context.sigmaZ(0, 3));
    }

    TEST(Scenarios_Pauli_MomentSimplifier, WrappingChain_Offset) {
        PauliContext context{10, WrapType::Wrap, SymmetryType::Translational};
        const auto& simplifier = context.moment_simplifier();
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_offset(context.sigmaX(0), qubit),
                      context.sigmaX(qubit)) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_offset(context.sigmaY(0), qubit),
                      context.sigmaY(qubit)) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_offset(context.sigmaZ(0), qubit),
                      context.sigmaZ(qubit)) << " qubit = " << qubit;
        }

        EXPECT_EQ(simplifier.chain_offset(context.sigmaX(0) * context.sigmaY(3), 2),
                  context.sigmaX(2) * context.sigmaY(5));

        EXPECT_EQ(simplifier.chain_offset(context.sigmaX(0) * context.sigmaY(3), 9),
                  context.sigmaX(9) * context.sigmaY(2));
    }

    TEST(Scenarios_Pauli_MomentSimplifier, WrappingLattice_Offset) {
        PauliContext context{4, 4, WrapType::Wrap, SymmetryType::Translational};
        const auto& simplifier = context.moment_simplifier();
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaX(0, 0), row, col),
                          context.sigmaX(row, col)) << " col = " << col << ", row = " << row;
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaY(0, 0), row, col),
                          context.sigmaY(row, col)) << " col = " << col << ", row = " << row;
                EXPECT_EQ(simplifier.lattice_offset(context.sigmaZ(0, 0), row, col),
                          context.sigmaZ(row, col)) << " col = " << col << ", row = " << row;
            }
        }

        EXPECT_EQ(simplifier.lattice_offset(context.sigmaX(0, 0) * context.sigmaZ(0, 3), 0, 1),
                  context.sigmaZ(0, 0) * context.sigmaX(0, 1));
    }

    TEST(Scenarios_Pauli_MomentSimplifier, NoWrapLattice_Supremum) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        MomentSimplifierNoWrappingLattice simplifier{context};
        EXPECT_EQ(simplifier.qubits, 16);
        EXPECT_EQ(simplifier.row_width, 4);
        EXPECT_EQ(simplifier.column_height, 4);
        EXPECT_EQ(simplifier.impl_label, MomentSimplifierNoWrappingLattice::expected_label);
        EXPECT_NE(simplifier.impl_label, MomentSimplifierNoWrappingChain::expected_label);

        EXPECT_EQ(simplifier.lattice_supremum(context.zero()), (std::pair<size_t, size_t>{0, 0}));
        EXPECT_EQ(simplifier.lattice_supremum(context.identity()),  (std::pair<size_t, size_t>{0, 0}));

        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(simplifier.lattice_supremum(context.sigmaX(row, col)),
                          (std::pair<size_t, size_t>{row + 1, col + 1})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_supremum(context.sigmaY(row, col)),
                          (std::pair<size_t, size_t>{row + 1, col + 1})) << " row = " << row << ", col = " << col;
                EXPECT_EQ(simplifier.lattice_supremum(context.sigmaZ(row, col)),
                          (std::pair<size_t, size_t>{row + 1, col + 1})) << " row = " << row << ", col = " << col;
            }
        }
        EXPECT_EQ(simplifier.lattice_supremum(context.sigmaX(0, 3) * context.sigmaY(1, 2)),
                  (std::pair<size_t, size_t>{2, 4}));
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