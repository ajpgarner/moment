/**
 * pauli_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/monomial_matrix.h"

#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_context.h"

#include "../../matrix/compare_os_matrix.h"
#include "../../matrix/compare_symbol_matrix.h"

#include <complex>
#include <set>

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

    TEST(Scenarios_Pauli_MatrixSystem, MomentMatrix_Qubit) {
        PauliMatrixSystem system{1};
        const auto& context = system.pauliContext;
        const auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1, Multithreading::MultiThreadPolicy::Never);

        // Compare operator sequences
        compare_mm_os_matrix(mmRaw, 4, {I, x, y, z,
                                        x, I, iz, miy,
                                        y, miz, I, ix,
                                        z, iy, mix, I});

        // Find symbols
        ASSERT_EQ(symbols.size(), 5); // 0, I, X, Y, Z

        const auto fX = symbols.where(x);
        ASSERT_TRUE(fX.found());
        auto sX = fX->Id();

        const auto fY = symbols.where(y);
        ASSERT_TRUE(fY.found());
        auto sY = fY->Id();

        const auto fZ = symbols.where(z);
        ASSERT_TRUE(fZ.found());
        const auto sZ = fZ->Id();

        std::set<symbol_name_t> symbol_set{0, 1, sX, sY, sZ};
        ASSERT_EQ(symbol_set.size(), 5);

        const std::complex<double> i{0.0, 1.0};

        compare_symbol_matrix(mmRaw, 4,
                              {Monomial{1, 1.0},  Monomial{sX, 1.0}, Monomial{sY, 1.0}, Monomial{sZ, 1.0},
                               Monomial{sX, 1.0}, Monomial{1, 1.0},  Monomial{sZ, i},   Monomial{sY, -i},
                               Monomial{sY, 1.0}, Monomial{sZ, -i},  Monomial{1, 1.0},  Monomial{sX, i},
                               Monomial{sZ, 1.0}, Monomial{sY, i},   Monomial{sX, -i},  Monomial{1, 1.0}});
    }

    TEST(Scenarios_Pauli_MatrixSystem, MomentMatrix_QubitMT) {
        PauliMatrixSystem system{1};
        const auto& context = system.pauliContext;
        const auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1, Multithreading::MultiThreadPolicy::Always);

        // Compare operator sequences
        compare_mm_os_matrix(mmRaw, 4, {I, x, y, z,
                                        x, I, iz, miy,
                                        y, miz, I, ix,
                                        z, iy, mix, I});

        // Find symbols
        ASSERT_EQ(symbols.size(), 5); // 0, I, X, Y, Z

        const auto fX = symbols.where(x);
        ASSERT_TRUE(fX.found());
        auto sX = fX->Id();

        const auto fY = symbols.where(y);
        ASSERT_TRUE(fY.found());
        auto sY = fY->Id();

        const auto fZ = symbols.where(z);
        ASSERT_TRUE(fZ.found());
        const auto sZ = fZ->Id();

        std::set<symbol_name_t> symbol_set{0, 1, sX, sY, sZ};
        ASSERT_EQ(symbol_set.size(), 5);

        const std::complex<double> i{0.0, 1.0};

        compare_symbol_matrix(mmRaw, 4,
                              {Monomial{1, 1.0},  Monomial{sX, 1.0}, Monomial{sY, 1.0}, Monomial{sZ, 1.0},
                               Monomial{sX, 1.0}, Monomial{1, 1.0},  Monomial{sZ, i},   Monomial{sY, -i},
                               Monomial{sY, 1.0}, Monomial{sZ, -i},  Monomial{1, 1.0},  Monomial{sX, i},
                               Monomial{sZ, 1.0}, Monomial{sY, i},   Monomial{sX, -i},  Monomial{1, 1.0}});
    }

}