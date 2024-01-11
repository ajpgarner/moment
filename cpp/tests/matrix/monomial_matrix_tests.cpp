/**
 * monomial_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "compare_os_matrix.h"
#include "compare_symbol_matrix.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

namespace Moment::Tests {
    TEST(Matrix_MonomialMatrix, PreMultiply) {
        Pauli::PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& factory = system.polynomial_factory();
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

        auto zMM_ptr = mmRaw.pre_multiply(Monomial{sZ, 1.0}, factory, symbols, Multithreading::MultiThreadPolicy::Never);
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
        Pauli::PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& factory = system.polynomial_factory();
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

        auto MMz_ptr = mmRaw.post_multiply(Monomial{sZ, 1.0}, factory, symbols, Multithreading::MultiThreadPolicy::Never);
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


    TEST(Matrix_MonomialMatrix, MultiplyByPolynomial) {

        // Make context with x, y
        Algebraic::AlgebraicMatrixSystem ams{
                std::make_unique<Algebraic::AlgebraicContext>(2)
        };
        const auto& context = ams.AlgebraicContext();
        auto& symbols = ams.Symbols();
        OperatorSequence x{{0}, context};
        OperatorSequence y{{1}, context};

        // Make all words up to length 3
        ams.generate_dictionary(3);

        // Make moment matrix
        const auto& mmRaw = ams.MomentMatrix(1);
        ASSERT_TRUE(mmRaw.is_monomial());
        const auto& mm = dynamic_cast<const MonomialMatrix&>(mmRaw);
        ASSERT_EQ(mm.Dimension(), 3);

        // Find symbols
        auto find_or_fail = [&symbols](OperatorSequence seq) -> Monomial {
            auto fX = symbols.where(seq);
            if (!fX.found()) {
                throw std::runtime_error(std::string("Did not find ") + seq.formatted_string());
            }
            return Monomial{fX->Id(), 1.0, fX.is_conjugated};
        };
        auto sX = find_or_fail(x);
        auto sY = find_or_fail(y);
        auto sXX = find_or_fail(OperatorSequence{{0, 0}, context});
        auto sXY = find_or_fail(OperatorSequence{{0, 1}, context});
        auto sYX = find_or_fail(OperatorSequence{{1, 0}, context});
        auto sYY = find_or_fail(OperatorSequence{{1, 1}, context});
        auto sXXX = find_or_fail(OperatorSequence{{0, 0, 0}, context});
        auto sXXY = find_or_fail(OperatorSequence{{0, 0, 1}, context});
        auto sXYX = find_or_fail(OperatorSequence{{0, 1, 0}, context});
        auto sXYY = find_or_fail(OperatorSequence{{0, 1, 1}, context});
        auto sYXX = find_or_fail(OperatorSequence{{1, 0, 0}, context});
        auto sYXY = find_or_fail(OperatorSequence{{1, 0, 1}, context});
        auto sYYX = find_or_fail(OperatorSequence{{1, 1, 0}, context});
        auto sYYY = find_or_fail(OperatorSequence{{1, 1, 1}, context});

        // Make polynomial
        const auto& factory = ams.polynomial_factory();
        const Polynomial x_plus_y = factory({sX, sY});
        ASSERT_EQ(x_plus_y.size(), 2);

        // Pre-multiply
        auto poly_mm_ptr = mm.pre_multiply(x_plus_y, factory, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_FALSE(poly_mm_ptr->is_monomial());
        const auto& poly_mm = dynamic_cast<const PolynomialMatrix&>(*poly_mm_ptr);
        compare_polynomial_matrix("(X + Y) * mm", poly_mm, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sX, sY}), factory({sXX, sYX}), factory({sXY, sYY}),
                                          factory({sXX, sYX}), factory({sXXX, sYXX}), factory({sXXY, sYXY}),
                                          factory({sXY, sYY}), factory({sXYX, sYYX}), factory({sXYY, sYYY})});

        // Post-multiply
        auto mm_poly_ptr = mm.post_multiply(x_plus_y, factory, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_FALSE(mm_poly_ptr->is_monomial());
        const auto& mm_poly = dynamic_cast<const PolynomialMatrix&>(*mm_poly_ptr);
        compare_polynomial_matrix("mm * (X + Y)", mm_poly, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sX, sY}), factory({sXX, sXY}), factory({sYX, sYY}),
                                          factory({sXX, sXY}), factory({sXXX, sXXY}), factory({sXYX, sXYY}),
                                          factory({sYX, sYY}), factory({sYXX, sYXY}), factory({sYYX, sYYY})});
    }

    TEST(Matrix_MonomialMatrix, MultiplyByZero) {
        // Make context with x, y
        Algebraic::AlgebraicMatrixSystem ams{
                std::make_unique<Algebraic::AlgebraicContext>(2)
        };
        const auto& context = ams.AlgebraicContext();
        const auto& factory = ams.polynomial_factory();
        auto& symbols = ams.Symbols();

        // Make moment matrix
        const auto& mmRaw = ams.MomentMatrix(1);
        ASSERT_TRUE(mmRaw.is_monomial());
        const auto& mm = dynamic_cast<const MonomialMatrix&>(mmRaw);
        ASSERT_EQ(mm.Dimension(), 3);

        auto poly_zero = Polynomial::Zero();
        ASSERT_TRUE(poly_zero.empty());

        auto zeroMMPtr = mm.pre_multiply(poly_zero, factory, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(zeroMMPtr->is_monomial());
        ASSERT_EQ(zeroMMPtr->Dimension(), 3);
        auto& zeroMM = dynamic_cast<const MonomialMatrix&>(*zeroMMPtr);
        for (size_t n = 0; n < 9; ++n) {
            EXPECT_EQ(zeroMM.raw_data()[n].id, 0) << n;
        }

        auto mmZeroPtr = mm.post_multiply(poly_zero, factory, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmZeroPtr->is_monomial());
        ASSERT_EQ(mmZeroPtr->Dimension(), 3);
        auto& mmZero = dynamic_cast<const MonomialMatrix&>(*mmZeroPtr);
        for (size_t n = 0; n < 9; ++n) {
            EXPECT_EQ(mmZero.raw_data()[n].id, 0) << n;
        }
    }

    TEST(Matrix_MonomialMatrix, AddMonomialMatrix) {
        // Make context with x, y
        Algebraic::AlgebraicMatrixSystem ams{
                std::make_unique<Algebraic::AlgebraicContext>(2)
        };
        const auto& context = ams.AlgebraicContext();
        const auto& factory = ams.polynomial_factory();
        auto& symbols = ams.Symbols();

        // Make moment matrix
        const auto& mmRaw = ams.MomentMatrix(1);
        ASSERT_TRUE(mmRaw.is_monomial());
        const auto& mm = dynamic_cast<const MonomialMatrix&>(mmRaw);
        ASSERT_EQ(mm.Dimension(), 3);

        // Make x localizing matrix
        const auto& lmXRaw = ams.LocalizingMatrix({1, OperatorSequence{{0}, context}});
        ASSERT_TRUE(lmXRaw.is_monomial());
        const auto& lmX = dynamic_cast<const MonomialMatrix&>(lmXRaw);
        ASSERT_EQ(lmXRaw.Dimension(), 3);

        // Find symbols
        auto find_or_fail = [&symbols](OperatorSequence seq) -> Monomial {
            auto fX = symbols.where(seq);
            if (!fX.found()) {
                throw std::runtime_error(std::string("Did not find ") + seq.formatted_string());
            }
            return Monomial{fX->Id(), 1.0, fX.is_conjugated};
        };
        auto sI = find_or_fail(OperatorSequence{{}, context});
        auto sX = find_or_fail(OperatorSequence{{0}, context});
        auto sY = find_or_fail(OperatorSequence{{1}, context});
        auto sXX = find_or_fail(OperatorSequence{{0, 0}, context});
        auto sXY = find_or_fail(OperatorSequence{{0, 1}, context});
        auto sYX = find_or_fail(OperatorSequence{{1, 0}, context});
        auto sYY = find_or_fail(OperatorSequence{{1, 1}, context});
        auto sXXX = find_or_fail(OperatorSequence{{0, 0, 0}, context});
        auto sXXY = find_or_fail(OperatorSequence{{0, 0, 1}, context});
        auto sYXX = find_or_fail(OperatorSequence{{1, 0, 0}, context});
        auto sYXY = find_or_fail(OperatorSequence{{1, 0, 1}, context});

        // Do mono+mono addition
        auto mmPlusLmX_ptr = mm.add(lmX, ams.polynomial_factory(), Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmPlusLmX_ptr->is_polynomial());
        auto& mmPlusLmX = dynamic_cast<PolynomialMatrix&>(*mmPlusLmX_ptr);

        compare_polynomial_matrix("mm + lmX", mmPlusLmX, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sI, sX}), factory({sX, sXX}), factory({sY, sXY}),
                                          factory({sX, sXX}), factory({sXX, sXXX}), factory({sXY, sXXY}),
                                          factory({sY, sYX}), factory({sYX, sYXX}), factory({sYY, sYXY})});

    }
}