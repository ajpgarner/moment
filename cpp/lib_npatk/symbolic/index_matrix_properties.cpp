/**
 * index_matrix_properties.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "index_matrix_properties.h"

#include "symbol_set.h"
#include "operators/moment_matrix.h"
namespace NPATK {

    IndexMatrixProperties::IndexMatrixProperties(const MomentMatrix &mm)
        : dimension{mm.dimension()} {

        size_t real_count = 0;
        size_t im_count = 0;
        for (const auto& us : mm.UniqueSequences) {
            const auto id = us.Id();

            // Skip 0 symbol
            if (id == 0) {
                continue;
            }

            std::pair<ptrdiff_t, ptrdiff_t> ri_index{real_count, im_count};

            // Moment matrix never pure imaginary, so register real symbol
            real_entries.emplace_back(id);
            ++real_count;

            // Also register imaginary symbol, if not a Hermitian element
            if (!us.is_hermitian()) {
                imaginary_entries.emplace_back(id);
                ++im_count;
            } else {
                ri_index.second = -1;
            }

            // Key
            this->elem_keys.emplace_hint(this->elem_keys.end(), std::make_pair(id, ri_index));
        }

        this->basis_type = (im_count > 0) ? MatrixType::Hermitian : MatrixType::Symmetric;
    }

    IndexMatrixProperties::IndexMatrixProperties(size_t dim, IndexMatrixProperties::MatrixType type, const SymbolSet& entries)
            : dimension(dim), basis_type(type) {
        size_t real_count = 0;
        size_t im_count = 0;
        for (const auto& [id, symbol] : entries.Symbols) {
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

