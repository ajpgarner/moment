/**
 * compare_os_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "gtest/gtest.h"

#include "matrix/operator_matrix/operator_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "matrix/operator_matrix/localizing_matrix.h"

#include <string>
#include <sstream>

namespace Moment::Tests {
    /**
     * @param prefix
     * @param theMM
     * @param dimension
     * @param reference NOTE: ROW-MAJOR REFERENCE LIST!
     */
    inline void compare_os_matrix(const std::string& prefix,
                                  const OperatorMatrix& theMM, const size_t dimension,
                                  const std::initializer_list<OperatorSequence> reference) {
        ASSERT_EQ(theMM.Dimension(), dimension) << prefix;
        size_t row = 0;
        size_t col = 0;
        for (const auto &ref_seq: reference) {
            ASSERT_LT(row, dimension) << prefix << ", row = " << row << ", col = " << col;
            ASSERT_LT(col, dimension) << prefix << ", row = " << row << ", col = " << col;

            const auto &actual_seq = theMM(std::array<size_t,2>{row, col});
            EXPECT_EQ(actual_seq, ref_seq) << prefix << ", row = " << row << ", col = " << col;
            ++col;
            if (col >= dimension) {
                col = 0;
                ++row;
            }
        }
        EXPECT_EQ(col, 0) << prefix;
        EXPECT_EQ(row, dimension) << prefix;
    }

    inline void compare_os_matrix(const SymbolicMatrix& symbolic, const size_t dimension,
                                  const std::initializer_list<OperatorSequence> reference) {
        ASSERT_TRUE(symbolic.has_operator_matrix());
        const auto& op_mat = symbolic.operator_matrix();
        compare_os_matrix(symbolic.Description(), op_mat, dimension, reference);
    }

    inline void compare_mm_os_matrix(const SymbolicMatrix& theMM, size_t dimension,
                                     const std::initializer_list<OperatorSequence> reference) {
        const auto* mmPtr = MomentMatrix::as_monomial_moment_matrix_ptr(theMM);
        ASSERT_NE(mmPtr, nullptr) << "Was not a monomial moment matrix!";


        std::string prefix{" Level = "};
        prefix += std::to_string(mmPtr->Level());
        return compare_os_matrix(prefix, *mmPtr, dimension, reference);
    }

    inline void compare_lm_os_matrix(const SymbolicMatrix& theLM, size_t dimension,
                                     const std::initializer_list<OperatorSequence> reference) {
        const auto* lmPtr = LocalizingMatrix::as_monomial_localizing_matrix_ptr(theLM);
        ASSERT_NE(lmPtr, nullptr) << "Was not a monomial localizing matrix!";

        std::stringstream ss;
        ss << " Level = " << lmPtr->Level() << ", word = " << theLM.context.format_sequence(lmPtr->Word());
        return compare_os_matrix(ss.str(), *lmPtr, dimension, reference);
    }

}