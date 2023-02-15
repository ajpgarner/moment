/**
 * symbolic_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbolic_matrix.h"

#include <stdexcept>

namespace Moment {


    SymbolicMatrix::SymbolicMatrix(const Context& context, SymbolTable& symbols,
                                   std::unique_ptr<SquareMatrix<SymbolExpression>> symbolMatrix)
        : context{context}, symbol_table{symbols}, Symbols{symbols}, SymbolMatrix{*this},
          dimension{symbolMatrix ? symbolMatrix->dimension : 0}, sym_exp_matrix{std::move(symbolMatrix)}
        {
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to SymbolicMatrix was nullptr."};
            }

            // Find included symbols
            std::set<symbol_name_t> included_symbols;
            const size_t max_symbol_id = symbols.size();
            for (const auto& x : *sym_exp_matrix) {
                assert(x.id < max_symbol_id);
                included_symbols.emplace(x.id);
            }

            // Create symbol matrix properties
            this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols));

    }

    void SymbolicMatrix::renumerate_bases(const SymbolTable &symbols) {
        for (auto& symbol : *this->sym_exp_matrix) {
            // Make conjugation status canonical:~
            if (symbol.conjugated) {
                const auto& ref_symbol = symbols[symbol.id];
                if (ref_symbol.is_hermitian()) {
                    symbol.conjugated = false;
                } else if (ref_symbol.is_antihermitian()) {
                    symbol.conjugated = false;
                    symbol.factor *= -1.0;
                }
            }
        }

        this->mat_prop->rebuild_keys(symbols);
    }

    SymbolicMatrix::~SymbolicMatrix() noexcept = default;
}