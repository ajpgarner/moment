/**
 * matrix_properties.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "matrix_properties.h"

#include "../matrix.h"
#include "symbolic/symbol_table.h"

#include <iostream>


namespace Moment {
    MatrixProperties::MatrixProperties(const Matrix& matrix, const SymbolTable& table,
                                       std::set<symbol_name_t>&& included, const std::string& desc,
                                       const bool complex_coefs, const bool is_herm)
            : dimension{matrix.Dimension()}, included_symbols{std::move(included)},
              mat_has_complex_coefficients{complex_coefs}, mat_is_herm{is_herm}, description(desc) {

        this->rebuild_keys(table);
    }

    void MatrixProperties::rebuild_keys(const SymbolTable& table) {

        // Reset real, imaginary and basis
        this->real_entries.clear();
        this->imaginary_entries.clear();
        this->elem_keys.clear();

        // Re-check every included symbol against symbol table
        for (const auto& id : this->included_symbols) {
            const auto& unique_symbol = table[id];
            assert(id == unique_symbol.Id());

            if (!unique_symbol.is_antihermitian()) {
                this->real_entries.insert(id);
            }
            if (!unique_symbol.is_hermitian()) {
                this->imaginary_entries.insert(id);
            }

            this->elem_keys.insert(this->elem_keys.end(), std::make_pair(id, unique_symbol.basis_key()));
        }

        // Complex, if matrix has complex coefficients or elements.
        this->mat_is_complex = this->mat_has_complex_coefficients || !this->imaginary_entries.empty();
    }

    std::ostream& operator<<(std::ostream& os, const MatrixProperties& mp) {
        os << mp.dimension << "x" << mp.dimension << " ";
        if (mp.IsComplex()) {
            if (mp.IsHermitian()) {
                os << "Hermitian matrix";
            } else {
                os << "Complex matrix";
            }
        } else {
            if (mp.IsHermitian()) {
                os << "Symmetric matrix";
            } else {
                os << "Real matrix";
            }
        }
        const auto num_us = mp.included_symbols.size();
        os << " with "
           << num_us << " unique " << (num_us != 1 ? "symbols" : "symbol");
        const auto num_re = mp.real_entries.size();
        if (num_re > 0) {
            os << ", " << num_re << " real";
        }
        const auto num_im = mp.imaginary_entries.size();
        if (num_im > 0) {
            os << ", " << num_im << " imaginary";
        }

        os << ".";

        return os;
    }

}