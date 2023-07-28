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

#include <vector>


namespace Moment::Tests {
    inline void compare_symbol_matrix(const Matrix &inputMM, size_t dimension,
                               const std::vector<Monomial>& reference) {
        const auto* mmPtr = MomentMatrix::as_monomial_moment_matrix_ptr(inputMM);
        ASSERT_NE(mmPtr, nullptr) << "Not a moment matrix!";
        ASSERT_TRUE(inputMM.is_monomial());
        const auto& theMM = dynamic_cast<const MonomialMatrix&>(inputMM);


        ASSERT_EQ(theMM.SymbolMatrix.Dimension(), dimension);

        size_t row = 0;
        size_t col = 0;
        for (const auto &ref_symbol: reference) {
            ASSERT_LT(row, dimension) << " Level = " << mmPtr->Level() << ", row = " << row << ", col = " << col;
            ASSERT_LT(col, dimension) << " Level = " << mmPtr->Level() << ", row = " << row << ", col = " << col;

            const auto &actual_symbol = theMM.SymbolMatrix(row, col);
            EXPECT_EQ(actual_symbol, ref_symbol)
                    << " Level = " << mmPtr->Level() << ", row = " << row << ", col = " << col;
            ++col;
            if (col >= dimension) {
                col = 0;
                ++row;
            }
        }
        EXPECT_EQ(col, 0) << " Level = " << mmPtr->Level();
        EXPECT_EQ(row, dimension) << " Level = " << mmPtr->Level();

    }

    inline void compare_symbol_matrix(const Matrix &theMM, size_t dimension,
                               std::initializer_list<std::string> reference) {
        std::vector<Monomial> txReference;
        txReference.reserve(reference.size());
        for (const auto& str : reference) {
            txReference.emplace_back(str);
        }
        compare_symbol_matrix(theMM, dimension, txReference);
    }
}