/**
 * symbolic_matrix_helpers.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "symbolic_matrix_helpers.h"

#include "symbolic/symbol_table.h"

#include "gtest/gtest.h"

#include <sstream>
#include <stdexcept>

namespace Moment::Tests {

    symbol_name_t find_or_fail(const SymbolTable& symbols, const OperatorSequence& seq) {
        const UniqueSequence * find_ptr = symbols.where(seq);
        if (find_ptr == nullptr) {
            std::stringstream ss;
            ss << "Could not find sequence \"" << seq << "\".";
            throw std::logic_error{ss.str()};
        }
        return find_ptr->Id();
    }

    void compare_symbol_matrices(const SymbolicMatrix::SymbolMatrixView& test,
                                 const std::vector<symbol_name_t>& reference) {
        ASSERT_EQ(test.Dimension()*test.Dimension(), reference.size());
        auto refIter = reference.cbegin();
        for (size_t row_counter = 0; row_counter < test.Dimension(); ++row_counter) {
            for (size_t column_counter = 0; column_counter < test.Dimension(); ++column_counter) {
                EXPECT_EQ(test[row_counter][column_counter].id, *refIter) << "row = " << row_counter
                                                                          << ", col = " << column_counter;
                ++refIter;
            }
        }
    }
}