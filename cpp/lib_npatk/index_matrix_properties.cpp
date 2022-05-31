/**
 * index_matrix_properties.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "index_matrix_properties.h"

#include "symbol_set.h"

namespace NPATK {

    IndexMatrixProperties::IndexMatrixProperties(size_t dim, IndexMatrixProperties::MatrixType type, SymbolSet&& entries)
            : dimension(dim), basis_type(type) {
        size_t real_count = 0;
        size_t im_count = 0;
        for (auto& [id, symbol] : entries.Symbols) {
            // Skip 0 symbol.
            if (id == 0) {
                continue;
            }

            std::pair<ptrdiff_t, ptrdiff_t> ri_index{real_count, im_count};

            if (!symbol.real_is_zero) {
                real_entries.emplace_back(symbol.id);
                ++real_count;
            } else {
                ri_index.first = -1;
            }

            if (!symbol.im_is_zero) {
                imaginary_entries.emplace_back(symbol.id);
                ++im_count;
            } else {
                ri_index.second = -1;
            }
            this->elem_keys.emplace_hint(this->elem_keys.end(), std::make_pair(symbol.id, ri_index));
        }

    }
}

