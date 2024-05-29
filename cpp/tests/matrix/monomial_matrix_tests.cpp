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

    TEST(Matrix_MonomialMatrix, Clone) {
        Pauli::PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& factory = system.polynomial_factory();
        auto& symbols = system.Symbols();

        // Create and check moment matrix
        const auto& [mm_index, mm] = system.MomentMatrix.create(1);
        ASSERT_EQ(mm.Dimension(), 4);
        ASSERT_TRUE(mm.is_monomial());
        const auto& mm_monomial = dynamic_cast<const MonomialMatrix&>(mm);

        // Clone matrix, and check it exists
        auto cloned_matrix = mm_monomial.clone(Multithreading::MultiThreadPolicy::Never);
        ASSERT_NE(cloned_matrix, nullptr);
        ASSERT_TRUE(cloned_matrix->is_monomial());
        const auto& cloned_monomial = dynamic_cast<const MonomialMatrix&>(*cloned_matrix);
        ASSERT_EQ(cloned_monomial.Dimension(), 4);

        // Check clone isn't just a pointer to original
        EXPECT_NE(&mm_monomial, &cloned_monomial);

        // Check data is identical, but not an alias
        const auto* ref_mat = mm_monomial.raw_data();
        ASSERT_NE(ref_mat, nullptr);
        const auto* test_mat = cloned_monomial.raw_data();
        ASSERT_NE(test_mat, nullptr);
        EXPECT_NE(ref_mat, test_mat);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const size_t index = col * 4 + row;
                EXPECT_EQ(test_mat[index], ref_mat[index]) << "[" << col << "," << row << "]";
            }
        }

        ASSERT_TRUE(mm_monomial.has_unaliased_operator_matrix());
        ASSERT_TRUE(cloned_monomial.has_unaliased_operator_matrix());

        const auto& ref_op_mat = mm_monomial.unaliased_operator_matrix();
        const auto& test_op_mat = cloned_monomial.unaliased_operator_matrix();
        ASSERT_EQ(ref_op_mat.Dimension(), 4);
        ASSERT_EQ(test_op_mat.Dimension(), 4);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                EXPECT_EQ(test_op_mat(row, col), ref_op_mat(row, col)) << "[" << col << "," << row << "]";
            }
        }
    }

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


        compare_os_matrix("Z*MM", zMM.unaliased_operator_matrix(),
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

    TEST(Matrix_MonomialMatrix, Multiply_Clone) {
        Pauli::PauliMatrixSystem system{std::make_unique<Pauli::PauliContext>(1)};
        const auto& context = system.pauliContext;
        const auto& factory = system.polynomial_factory();
        auto& symbols = system.Symbols();

        // Produce moment matrix
        const auto& mm = system.MomentMatrix(1);
        ASSERT_EQ(mm.Dimension(), 4);
        ASSERT_TRUE(mm.is_monomial());
        const auto& mm_monomial = dynamic_cast<const MonomialMatrix&>(mm);

        // Make scalar
        const Monomial one{1, 1.0, false};

        auto mult_ptr = mm_monomial.pre_multiply(one, factory, symbols, Multithreading::MultiThreadPolicy::Never);
        ASSERT_NE(mult_ptr, nullptr);
        ASSERT_TRUE(mult_ptr->is_monomial());
        const auto& cloned_monomial = dynamic_cast<const MonomialMatrix&>(*mult_ptr);
        ASSERT_EQ(cloned_monomial.Dimension(), 4);
        EXPECT_EQ(cloned_monomial.global_factor(), std::complex<double>(1.0, 0));


        // Check data is identical, but not an alias
        const auto* ref_mat = mm_monomial.raw_data();
        ASSERT_NE(ref_mat, nullptr);
        const auto* test_mat = cloned_monomial.raw_data();
        ASSERT_NE(test_mat, nullptr);
        EXPECT_NE(ref_mat, test_mat);
        for (size_t col = 0; col < 4; ++col) {
            for (size_t row = 0; row < 4; ++row) {
                const size_t index = col * 4 + row;
                EXPECT_EQ(test_mat[index], ref_mat[index]) << "[" << col << "," << row << "]";
            }
        }


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
        compare_os_matrix("MM*z", MMz.unaliased_operator_matrix(),
                          4, {z, miy, ix , I,
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


    TEST(Matrix_MonomialMatrix, AddMonomial) {
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

        // Do mono+mono addition
        auto mmPlusX_ptr = mm.add(sX, ams.polynomial_factory(), Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmPlusX_ptr->is_polynomial());
        auto& mmPlusLmX = dynamic_cast<PolynomialMatrix&>(*mmPlusX_ptr);

        compare_polynomial_matrix("mm + X", mmPlusLmX, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sI, sX}), factory({sX, sX}), factory({sY, sX}),
                                          factory({sX, sX}), factory({sXX, sX}), factory({sXY, sX}),
                                          factory({sY, sX}), factory({sYX, sX}), factory({sYY, sX})});

    }


    TEST(Matrix_MonomialMatrix, AddMonomialZero) {
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

        // Do mono + mono zero addition
        auto mmPlusZero_ptr = mm.add(Monomial{0}, ams.polynomial_factory(), Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmPlusZero_ptr->is_polynomial());
        auto& mmPlusZero = dynamic_cast<PolynomialMatrix&>(*mmPlusZero_ptr);

        compare_polynomial_matrix("mm + 0", mmPlusZero, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sI}), factory({sX}), factory({sY}),
                                          factory({sX}), factory({sXX}), factory({sXY}),
                                          factory({sY}), factory({sYX}), factory({sYY})});

    }

    TEST(Matrix_MonomialMatrix, AddPolynomial) {
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

        // Do mono + poly addition
        Polynomial poly = factory({sX, sY});

        auto mmPlusX_ptr = mm.add(poly, ams.polynomial_factory(), Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmPlusX_ptr->is_polynomial());
        auto& mmPlusLmX = dynamic_cast<PolynomialMatrix&>(*mmPlusX_ptr);

        compare_polynomial_matrix("mm + X + Y", mmPlusLmX, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sI, sX, sY}), factory({sX, sX, sY}), factory({sY, sX, sY}),
                                          factory({sX, sX, sY}), factory({sXX, sX, sY}), factory({sXY, sX, sY}),
                                          factory({sY, sX, sY}), factory({sYX, sX, sY}), factory({sYY, sX, sY})});

    }

    TEST(Matrix_MonomialMatrix, AddPolynomialZero) {
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

        // Do mono + poly 0
        auto mmPlusZero_ptr = mm.add(Polynomial::Zero(), ams.polynomial_factory(), Multithreading::MultiThreadPolicy::Never);
        ASSERT_TRUE(mmPlusZero_ptr->is_polynomial());
        auto& mmPlusZero = dynamic_cast<PolynomialMatrix&>(*mmPlusZero_ptr);

        compare_polynomial_matrix("mm + 0", mmPlusZero, 3, factory.zero_tolerance,
                                  std::vector<Polynomial>{
                                          factory({sI}), factory({sX}), factory({sY}),
                                          factory({sX}), factory({sXX}), factory({sXY}),
                                          factory({sY}), factory({sYX}), factory({sYY})});

    }
}