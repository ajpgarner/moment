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

    class SymbolComboToBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit SymbolComboToBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<basis_vec_t, basis_vec_t> operator()(const SymbolCombo& combo) const;
    };

    class BasisVecToSymbolCombo {
    public:
        const SymbolTable& symbols;

    public:
        explicit BasisVecToSymbolCombo(const SymbolTable& symbols) : symbols{symbols} { }

        SymbolCombo operator()(const basis_vec_t& real, const basis_vec_t& img) const;
    };



}