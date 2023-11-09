/**
 * monomial_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "compare_os_matrix.h"
#include "compare_symbol_matrix.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

namespace Moment::Tests {
    TEST(Matrix_MonomialMatrix, PreMultiply) {
        Pauli::PauliMatrixSystem system{1};
        const auto& context = system.pauliContext;
        auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto i_os = OperatorSequence{{}, context, SequenceSignType::Imaginary};
        auto mi = OperatorSequence{{}, context, SequenceSignType::NegativeImaginary};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto mx = context.sigmaX(0, SequenceSignType::Negative);
        auto my = context.sigmaY(0, SequenceSignType::Negative);
        auto mz = context.sigmaZ(0, SequenceSignType::Negative);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1);
        ASSERT_EQ(mmRaw.Dimension(), 4);

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

        auto zMM_ptr = mmRaw.pre_multiply(Monomial{sZ, 1.0}, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(zMM_ptr);
        ASSERT_TRUE(zMM_ptr->is_monomial());
        const auto& zMM = dynamic_cast<MonomialMatrix&>(*zMM_ptr);


        compare_os_matrix("Z*MM", zMM.operator_matrix(),
                          4, {z, iy, mix, I,
                              iy, z, i_os, mx,
                              mix, mi, z, my,
                              I, x, y, z});

        const std::complex<double> i{0, 1.0};
        compare_monomial_matrix("Z*MM", zMM, 4,
                               {Monomial{sZ, 1.0},  Monomial{sY, i}, Monomial{sX, -i}, Monomial{1, 1.0},
                                Monomial{sY, i}, Monomial{sZ, 1.0},  Monomial{1, i},   Monomial{sX, -1.0},
                                Monomial{sX, -i}, Monomial{1, -i},  Monomial{sZ, 1.0},  Monomial{sY, -1.0},
                                Monomial{1, 1.0}, Monomial{sX, 1.0},   Monomial{sY, 1.0},  Monomial{sZ, 1.0}});
    }

    TEST(Matrix_MonomialMatrix, PostMultiply) {
        Pauli::PauliMatrixSystem system{1};
        const auto& context = system.pauliContext;
        auto& symbols = system.Symbols();

        auto I = OperatorSequence{context};
        auto i_os = OperatorSequence{{}, context, SequenceSignType::Imaginary};
        auto mi = OperatorSequence{{}, context, SequenceSignType::NegativeImaginary};
        auto x = context.sigmaX(0);
        auto y = context.sigmaY(0);
        auto z = context.sigmaZ(0);
        auto mx = context.sigmaX(0, SequenceSignType::Negative);
        auto my = context.sigmaY(0, SequenceSignType::Negative);
        auto mz = context.sigmaZ(0, SequenceSignType::Negative);
        auto ix = context.sigmaX(0, SequenceSignType::Imaginary);
        auto iy = context.sigmaY(0, SequenceSignType::Imaginary);
        auto iz = context.sigmaZ(0, SequenceSignType::Imaginary);
        auto mix = context.sigmaX(0, SequenceSignType::NegativeImaginary);
        auto miy = context.sigmaY(0, SequenceSignType::NegativeImaginary);
        auto miz = context.sigmaZ(0, SequenceSignType::NegativeImaginary);

        // Produce moment matrix
        const auto& mmRaw = system.MomentMatrix(1);
        ASSERT_EQ(mmRaw.Dimension(), 4);

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

        auto MMz_ptr = mmRaw.post_multiply(Monomial{sZ, 1.0}, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(MMz_ptr);
        ASSERT_TRUE(MMz_ptr->is_monomial());
        const auto& MMz = dynamic_cast<MonomialMatrix&>(*MMz_ptr);

        // Compare operator sequences
        compare_os_matrix("MM*z", MMz.operator_matrix(), 4, {z, miy, ix , I,
                                                             miy, z, i_os, x,
                                                             ix, mi, z, y,
                                                             I, mx, my, z});

        const std::complex<double> i{0, 1.0};
        compare_monomial_matrix("MM*Z", MMz, 4,
                                {Monomial{sZ, 1.0},  Monomial{sY, -i}, Monomial{sX, i}, Monomial{1, 1.0},
                                 Monomial{sY, -i}, Monomial{sZ, 1.0},  Monomial{1, i},   Monomial{sX, 1.0},
                                 Monomial{sX, i}, Monomial{1, -i},  Monomial{sZ, 1.0},  Monomial{sY, 1.0},
                                 Monomial{1, 1.0}, Monomial{sX, -1.0},   Monomial{sY, -1.0},  Monomial{sZ, 1.0}});
    }
}