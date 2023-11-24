/**
 * order_symbols_by_hash.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "monomial_comparator_by_hash.h"

#include "symbol_table.h"

namespace Moment {

    bool CompareByOpHash::operator()(symbol_name_t lhs, symbol_name_t rhs) const noexcept {
        auto key_lhs = this->key(Monomial{lhs});
        auto key_rhs = this->key(Monomial{rhs});
        return key_lhs < key_rhs;
    }

    bool CompareByOpHash::operator()(const Monomial& lhs, const Monomial& rhs) const noexcept {
        auto key_lhs = this->key(lhs);
        auto key_rhs = this->key(rhs);
        return key_lhs < key_rhs;
    }

    std::pair<uint64_t, uint64_t> CompareByOpHash::key(const Monomial& monomial) const noexcept {
        assert(monomial.id < this->symbolTable.size());
        const auto& lhs_entry = this->symbolTable[monomial.id];
        if (lhs_entry.has_sequence()) {
            return {lhs_entry.hash(), static_cast<uint64_t>(monomial.conjugated ? 1 : 0)};
        }

        uint64_t tx_id = static_cast<uint64_t>(monomial.id)*2 + static_cast<uint64_t>(monomial.conjugated ? 1 : 0);
        return {std::numeric_limits<uint64_t>::max(), tx_id};
    }

    size_t ByHashPolynomialFactory::maximum_degree(const Polynomial& poly) const {
        // Iterate backwards, and return degree of first defined symbol
        auto rev_iter = poly.rbegin();
        const auto rev_iter_end = poly.rend();
        while (rev_iter != rev_iter_end) {
            auto id = rev_iter->id;
            if (id <= 1) {
                return 0;
            }
            assert(id < this->symbols.size());
            const auto& symbol = this->symbols[id];
            if (symbol.has_sequence()) {
                return symbol.sequence().size();
            }
        }

        // Nothing found; 0 degree
        return 0;
    }
}