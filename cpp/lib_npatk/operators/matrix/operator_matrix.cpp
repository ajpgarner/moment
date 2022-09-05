/**
 * operator_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_matrix.h"
#include "symbol_table.h"

namespace NPATK {
    OperatorMatrix::SymbolMatrixProperties::SymbolMatrixProperties(const OperatorMatrix& opMat,
                                                                   const SymbolTable& table,
                                                                   std::set<symbol_name_t>&& included)
           : matrix{opMat}, included_symbols{std::move(included)}  {
        size_t real_count = 0;
        size_t im_count = 0;
        for (const auto& id : included_symbols) {
            auto& us = table[id];

            // Skip 0 symbol
            if (id == 0) {
                continue;
            }

            std::pair<ptrdiff_t, ptrdiff_t> ri_index{real_count, im_count};

            // Moment matrix never pure imaginary, so register real symbol
            this->real_entries.emplace_back(id);
            ++real_count;

            // Also register imaginary symbol, if not a Hermitian element
            if (!us.is_hermitian()) {
                this->imaginary_entries.emplace_back(id);
                ++im_count;
            } else {
                ri_index.second = -1;
            }

            // Key
            this->elem_keys.emplace_hint(this->elem_keys.end(), std::make_pair(id, ri_index));
        }

        // Get type, based on number of imaginary elements
        this->basis_type = (im_count > 0) ? MatrixType::Hermitian : MatrixType::Symmetric;
    }
}