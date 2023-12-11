/**
 * nonwrapping_simplifier_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/nonwrapping_simplifier.h"

#include <array>
#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_NonwrappingSimplifier, ChainEmpty) {
        PauliContext empty{0};
        NonwrappingChainSimplifier simplifier{empty};
        EXPECT_EQ(simplifier.qubits, 0);
    }

    TEST(Scenarios_Pauli_NonwrappingSimplifier, LatticeEmpty) {
        PauliContext empty{0};
        NonwrappingLatticeSimplifier simplifier{empty};
        EXPECT_EQ(simplifier.qubits, 0);
    }

    TEST(Scenarios_Pauli_NonwrappingSimplifier, ChainMinimum) {
        PauliContext context{10, WrapType::None, SymmetryType::Translational};
        NonwrappingChainSimplifier simplifier{context};
        EXPECT_EQ(simplifier.impl_label, NonwrappingChainSimplifier::expected_label);
        EXPECT_NE(simplifier.impl_label, NonwrappingLatticeSimplifier::expected_label);

        EXPECT_EQ(simplifier.chain_minimum(context.zero()), 0);
        EXPECT_EQ(simplifier.chain_minimum(context.identity()), 0);

        EXPECT_EQ(simplifier.qubits, 10);
        for (size_t qubit = 0; qubit < 10; ++qubit) {
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaX(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaY(qubit)), qubit) << " qubit = " << qubit;
            EXPECT_EQ(simplifier.chain_minimum(context.sigmaZ(qubit)), qubit) << " qubit = " << qubit;
        }
    }

    TEST(Scenarios_Pauli_NonwrappingSimplifier, LatticeMinimum) {
        PauliContext context{4, 4, WrapType::None, SymmetryType::Translational};
        NonwrappingLatticeSimplifier simplifier{context};
        EXPECT_EQ(simplifier.qubits, 16);
        EXPECT_EQ(simplifier.row_width, 4);
        EXPECT_EQ(simplifier.column_height, 4);
        EXPECT_EQ(simplifier.impl_label, NonwrappingLatticeSimplifier::expected_label);
        EXPECT_NE(simplifier.impl_label, NonwrappingChainSimplifier::expected_label);

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
    }
}