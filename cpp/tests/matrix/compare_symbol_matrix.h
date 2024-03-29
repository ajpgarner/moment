/**
 * compare_symbol_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "gtest/gtest.h"

#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/polynomial_factory.h"

#include <vector>


namespace Moment::Tests {


    inline void compare_monomial_matrix(const std::string& prefix,
                                        const MonomialMatrix& theMM, const size_t dimension,
                                        const std::vector<Monomial>& reference) {

        ASSERT_EQ(theMM.SymbolMatrix.Dimension(), dimension);

        size_t row = 0;
        size_t col = 0;
        for (const auto &ref_symbol: reference) {
            ASSERT_LT(row, dimension) << " " << prefix << ", row = " << row << ", col = " << col;
            ASSERT_LT(col, dimension) << " " << prefix << ", row = " << row << ", col = " << col;

            const auto &actual_symbol = theMM.SymbolMatrix(row, col);
            EXPECT_EQ(actual_symbol, ref_symbol)
                                << " " << prefix << ", row = " << row << ", col = " << col;
            ++col;
            if (col >= dimension) {
                col = 0;
                ++row;
            }
        }
        EXPECT_EQ(col, 0) << " " << prefix;
        EXPECT_EQ(row, dimension) << " " << prefix;
    }

    /**
     * @param inputMM
     * @param dimension
     * @param reference Warning: row major!
     */
    inline void compare_symbol_matrix(const SymbolicMatrix &inputMM, const size_t dimension,
                                      const std::vector<Monomial>& reference) {
        ASSERT_TRUE(inputMM.is_monomial());
        MomentMatrix const * mmPtr = MomentMatrix::to_operator_matrix_ptr(inputMM);
        if (mmPtr != nullptr) {
            const auto& theMM = dynamic_cast<const MonomialMatrix&>(inputMM);

            compare_monomial_matrix(std::string("Level = ") + std::to_string(mmPtr->Index),
                                    theMM, dimension, reference);
        } else {
            compare_monomial_matrix(inputMM.Description(), dynamic_cast<const MonomialMatrix&>(inputMM),
                                    dimension, reference);
        }


    }

    inline void compare_polynomial_matrix(const std::string& prefix,
                                          const PolynomialMatrix& testMatrix, const size_t dimension,
                                          const double zero_tolerance,
                                          const std::vector<Polynomial>& reference) {

        ASSERT_EQ(testMatrix.Dimension(), dimension);

        size_t row = 0;
        size_t col = 0;
        for (const auto& ref_polynomial: reference) {
            ASSERT_LT(row, dimension) << " " << prefix << ", row = " << row << ", col = " << col;
            ASSERT_LT(col, dimension) << " " << prefix << ", row = " << row << ", col = " << col;
            const auto& actual_polynomial = testMatrix.SymbolMatrix(row, col);

            ASSERT_EQ(actual_polynomial.size(), ref_polynomial.size())
                                << " " << prefix << ", row = " << row << ", col = " << col
                                << "\n actual = " << actual_polynomial << ",\n reference = " << ref_polynomial;
            for (size_t pN = 0; pN < ref_polynomial.size(); ++pN) {
                EXPECT_EQ(actual_polynomial[pN].id, ref_polynomial[pN].id)
                                    << " " << prefix << ", row = " << row << ", col = " << col << ", elem = " << pN
                                    << "\n actual = " << actual_polynomial << ",\n reference = " << ref_polynomial;
                EXPECT_EQ(actual_polynomial[pN].conjugated, ref_polynomial[pN].conjugated)
                                    << " " << prefix << ", row = " << row << ", col = " << col << ", elem = " << pN
                                    << "\n actual = " << actual_polynomial << ",\n reference = " << ref_polynomial;
                EXPECT_TRUE(approximately_equal(actual_polynomial[pN].factor, ref_polynomial[pN].factor, zero_tolerance))
                                    << " " << prefix << ", row = " << row << ", col = " << col << ", elem = " << pN
                                    << "\n actual = " << actual_polynomial << ",\n reference = " << ref_polynomial;
            }
            ++col;
            if (col >= dimension) {
                col = 0;
                ++row;
            }
        }
        EXPECT_EQ(col, 0) << " " << prefix;
        EXPECT_EQ(row, dimension) << " " << prefix;
    }

    inline void compare_symbol_matrix(const SymbolicMatrix &theMM, size_t dimension,
                                      std::initializer_list<std::string> reference) {
        std::vector<Monomial> txReference;
        txReference.reserve(reference.size());
        for (const auto& str : reference) {
            txReference.emplace_back(str);
        }
        compare_symbol_matrix(theMM, dimension, txReference);
    }
}