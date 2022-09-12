/**
 * symbol_matrix_properties.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_matrix_properties.h"

#include "operator_matrix.h"
#include "symbol_table.h"
#include "symbolic/symbol_set.h"


namespace NPATK {
    SymbolMatrixProperties::SymbolMatrixProperties(const OperatorMatrix& matrix,
                                                   const SymbolTable& table,
                                                   std::set<symbol_name_t>&& included)
            : dimension{matrix.Dimension()}, included_symbols{std::move(included)}  {

        // Sort symbols into real only, and complex
        for (const auto& id : included_symbols) {
            const auto& unique_symbol = table[id];
            assert(id == unique_symbol.Id());

            this->real_entries.insert(id);
            if (!unique_symbol.is_hermitian()) {
                this->imaginary_entries.insert(id);
            }

            // Copy key from symbol table into local table
            this->elem_keys.insert(this->elem_keys.end(), std::make_pair(id, unique_symbol.basis_key()));
        }

        // Matrix type depends on whether there are imaginary symbols
        this->basis_type = this->imaginary_entries.empty() ? MatrixType::Symmetric : MatrixType::Hermitian;
    }


    SymbolMatrixProperties::SymbolMatrixProperties(size_t dim, MatrixType type, const SymbolSet &entries)
        :  dimension{dim}, basis_type{type} {

        ptrdiff_t real_count = 0;
        ptrdiff_t im_count = 0;

        // Go through symbol table... [should be sorted...]
        for (const auto& [id, symbol] : entries.Symbols) {
            // Skip 0 symbol.
            if (id == 0) {
                continue;
            }

            ptrdiff_t re_index = -1;
            ptrdiff_t im_index = -1;

            if (!symbol.real_is_zero) {
                re_index = real_count++;
                this->real_entries.insert(real_entries.end(), symbol.id);
            }

            if (!symbol.im_is_zero) {
                im_index = im_count++;
                this->imaginary_entries.insert(imaginary_entries.end(), symbol.id);
            }

            // Inferred basis maps, ignoring 0 element.
            this->elem_keys.insert(this->elem_keys.end(),
                                   std::make_pair(symbol.id, std::make_pair(re_index, im_index)));
        }

        // Unknown matrix type depends on whether there are imaginary symbols
        if (this->basis_type == MatrixType::Unknown) {
            this->basis_type = this->imaginary_entries.empty() ? MatrixType::Symmetric : MatrixType::Hermitian;
        }
    }
}