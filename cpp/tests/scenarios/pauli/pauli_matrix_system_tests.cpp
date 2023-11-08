/**
 * pauli_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_context.h"

namespace Moment::Tests {
    using namespace Moment::Pauli;

    TEST(Scenarios_Pauli_MatrixSystem, Construct_Empty) {
        PauliMatrixSystem system{0};
        const auto& context = system.pauliContext;
        EXPECT_EQ(context.size(), 0);
        EXPECT_EQ(context.qubit_size, 0);
    }

    TEST(Scenarios_Pauli_MatrixSystem, Construct_TwoQubit) {
        PauliMatrixSystem system{2};
        const auto& context = system.pauliContext;
        EXPECT_EQ(context.size(), 6);
        EXPECT_EQ(context.qubit_size, 2);
    }
}