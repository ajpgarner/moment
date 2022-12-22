/**
 * compare_os_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "gtest/gtest.h"

#include "matrix/operator_matrix.h"
#include "matrix/moment_matrix.h"
#include "matrix/localizing_matrix.h"

#include <sstream>

namespace Moment::Tests {
    inline void compare_os_matrix(const std::string& prefix,
                                  const OperatorMatrix& theMM, size_t dimension,
                                  const std::initializer_list<OperatorSequence> reference) {
        ASSERT_EQ(theMM.Dimension(), dimension) << prefix;
        size_t row = 0;
        size_t col = 0;
        for (const auto &ref_seq: reference) {
            ASSERT_LT(row, dimension) << prefix << ", row = " << row << ", col = " << col;
            ASSERT_LT(col, dimension) << prefix << ", row = " << row << ", col = " << col;

            const auto &actual_seq = theMM.SequenceMatrix[row][col];
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

    inline void compare_mm_os_matrix(const MomentMatrix& theMM, size_t dimension,
                                     const std::initializer_list<OperatorSequence> reference) {
        std::string prefix{" Level = "};
        prefix += std::to_string(theMM.Level());
        return compare_os_matrix(prefix, theMM, dimension, reference);
    }

    inline void compare_lm_os_matrix(const LocalizingMatrix& theLM, size_t dimension,
                                     const std::initializer_list<OperatorSequence> reference) {
        std::stringstream ss;
        ss << " Level = " << theLM.Level() << ", word = " << theLM.context.format_sequence(theLM.Word());
        return compare_os_matrix(ss.str(), theLM, dimension, reference);
    }

}