/**
 * symbol_combo_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/SparseCore>

#include "symbol_combo.h"

namespace Moment {
    class SymbolTable;
    class SymbolCombo;

    using basis_vec_t = Eigen::SparseVector<double>;
    using complex_basis_vec_t = Eigen::SparseVector<std::complex<double>>;

    /**
     * Convert a SymbolCombo into a vector of basis co-efficients.
     */
    class SymbolComboToBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit SymbolComboToBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<basis_vec_t, basis_vec_t> operator()(const SymbolCombo& combo) const;
    };

    /**
     * Convert a SymbolCombo into a vector of complex basis co-efficients.
     */
    class SymbolComboToComplexBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit SymbolComboToComplexBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<complex_basis_vec_t, complex_basis_vec_t> operator()(const SymbolCombo& combo) const;
    };

    /**
     * Convert a vector of basis co-efficients into a SymbolCombo.
     */
    class BasisVecToSymbolCombo {
    public:
        const SymbolTable& symbols;

    public:
        explicit BasisVecToSymbolCombo(const SymbolTable& symbols) : symbols{symbols} { }

        SymbolCombo operator()(const basis_vec_t& real, const basis_vec_t& img) const;
    };

    /**
     * Convert a vector of complex basis co-efficients into a SymbolCombo.
     */
    class ComplexBasisVecToSymbolCombo {
    public:
        const SymbolTable& symbols;

    public:
        explicit ComplexBasisVecToSymbolCombo(const SymbolTable& symbols) : symbols{symbols} { }

        SymbolCombo operator()(const complex_basis_vec_t& real, const complex_basis_vec_t& img) const;
    };



}