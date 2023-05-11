/**
 * compare_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <vector>

namespace Moment::Tests {

    template<typename matrix_t>
    inline void assert_same_matrix(const std::string &name, const matrix_t& test, const matrix_t& ref) {
        ASSERT_EQ(test.cols(), ref.cols()) << name;
        ASSERT_EQ(test.rows(), ref.rows()) << name;
        for (int col_index = 0; col_index < ref.cols(); ++col_index) {
            for (int row_index = 0; row_index < ref.rows(); ++row_index) {
                EXPECT_EQ(test.coeff(row_index, col_index), ref.coeff(row_index, col_index))
                        << name << ": (" << row_index << ", " << col_index << ")";
            }
        }
    }

    template<typename matrix_t>
    inline void assert_same_basis(const std::string &name,
                           const std::vector<matrix_t>& test, const std::vector<matrix_t>& ref) {
        ASSERT_EQ(test.size(), ref.size()) << name;
        for (size_t elem = 0; elem < test.size(); ++elem) {
            std::stringstream ss;
            ss << name << " #" << elem;
            assert_same_matrix(ss.str(), test[elem], ref[elem]);
        }
    }
}