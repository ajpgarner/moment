/**
 * symbolic_matrix_helpers.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbolic_matrix_helpers.h"

#include "matrix/polynomial_matrix.h"
#include "symbolic/symbol_table.h"

#include "gtest/gtest.h"

#include <sstream>
#include <stdexcept>

namespace Moment::Tests {

    symbol_name_t find_or_fail(const SymbolTable& symbols, const OperatorSequence& seq) {
        const auto find_ptr = symbols.where(seq);
        if (!find_ptr.found()) {
            std::stringstream ss;
            ss << "Could not find sequence \"" << seq << "\".";
            throw std::logic_error{ss.str()};
        }
        return find_ptr->Id();
    }

    void compare_symbol_matrices(const Matrix &test, const Matrix &reference,const std::string& label) {
        ASSERT_EQ(test.is_monomial(), reference.is_monomial()) << label;

        if (reference.is_monomial()) {
            compare_symbol_matrices(dynamic_cast<const MonomialMatrix&>(test),
                                    dynamic_cast<const MonomialMatrix&>(reference),
                                    label);
        } else {
            compare_symbol_matrices(dynamic_cast<const PolynomialMatrix&>(test),
                                    dynamic_cast<const PolynomialMatrix&>(reference),
                                    label);
        }
    }

    void compare_symbol_matrices(const MonomialMatrix &test, const MonomialMatrix &reference,
                                 const std::string& label) {
        ASSERT_EQ(test.Dimension(), reference.Dimension()) << label;
        EXPECT_EQ(test.HasComplexCoefficients(), reference.HasComplexCoefficients()) << label;
        EXPECT_EQ(test.Hermitian(), reference.Hermitian()) << label;

        for (size_t rIdx = 0; rIdx < reference.Dimension(); ++rIdx) {
            for (size_t cIdx = 0; cIdx < reference.Dimension(); ++cIdx) {
                const auto& test_elem = test.SymbolMatrix[rIdx][cIdx];
                const auto& ref_elem = reference.SymbolMatrix[rIdx][cIdx];
                EXPECT_EQ(test_elem, ref_elem) << label << (label.empty() ? "" : ", ") << "row = " << rIdx << ", col = " << cIdx;
            }
        }
    }

    void compare_symbol_matrices(const PolynomialMatrix &test,
                                 const PolynomialMatrix &reference,
                                 const std::string& label) {
        ASSERT_EQ(test.Dimension(), reference.Dimension()) << label;
        EXPECT_EQ(test.HasComplexCoefficients(), reference.HasComplexCoefficients()) << label;
        EXPECT_EQ(test.Hermitian(), reference.Hermitian()) << label;

        for (size_t rIdx = 0; rIdx < reference.Dimension(); ++rIdx) {
            for (size_t cIdx = 0; cIdx < reference.Dimension(); ++cIdx) {
                const auto& test_elem = test.SymbolMatrix[rIdx][cIdx];
                const auto& ref_elem = reference.SymbolMatrix[rIdx][cIdx];
                EXPECT_EQ(test_elem, ref_elem)
                    << label << (label.empty() ? "" : ", ") << "row = " << rIdx << ", col = " << cIdx;
            }
        }
    }

    void compare_symbol_matrices(const Matrix& test,
                                 const std::vector<symbol_name_t>& reference, const std::string& label) {
        ASSERT_TRUE(test.is_monomial()) << label;
        const auto& test_mm = dynamic_cast<const MonomialMatrix&>(test);
        compare_symbol_matrices(test_mm.SymbolMatrix, reference, label);
    }

    void compare_symbol_matrices(const MonomialMatrix::MMSymbolMatrixView& test,
                                 const std::vector<symbol_name_t>& reference,
                                 const std::string& label) {
        ASSERT_EQ(test.Dimension()*test.Dimension(), reference.size()) << label;
        auto refIter = reference.cbegin();
        for (size_t row_counter = 0; row_counter < test.Dimension(); ++row_counter) {
            for (size_t column_counter = 0; column_counter < test.Dimension(); ++column_counter) {
                EXPECT_EQ(test[row_counter][column_counter].id, *refIter) << label << (label.empty() ? "" : ", ") << "row = " << row_counter
                                                                          << ", col = " << column_counter;
                ++refIter;
            }
        }
    }
}