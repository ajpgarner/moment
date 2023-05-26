/**
 * order_symbols_by_hash.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "order_symbols_by_hash.h"

namespace Moment {

    bool CompareByOpHash::operator()(const Monomial& lhs, const Monomial& rhs) const noexcept {
        assert(lhs.id < this->symbolTable.size());
        assert(rhs.id < this->symbolTable.size());

        const auto& lhs_entry = this->symbolTable[lhs.id];
        const auto& rhs_entry = this->symbolTable[rhs.id];

        // Can we directly compare?
        if (lhs_entry.has_sequence() && rhs_entry.has_sequence()) {
            const auto lhs_hash = lhs_entry.hash();
            const auto rhs_hash = rhs_entry.hash();

            if (lhs_hash < rhs_hash) {
                return true;
            } else if ((lhs_hash > rhs_hash) || (lhs.conjugated == rhs.conjugated)) {
                return false;
            }

            return !lhs.conjugated; // true implies LHS a, RHS a*
        }

        // Indirect comparison required (e.g. because we have an extended symbol table).

        if (lhs_entry.has_sequence()) { // implicit, RHS no sequence
            return true; // All sequenced entries are before unsequenced entries.
        }
        if (rhs_entry.has_sequence()) { // implicit, LHS no sequence
            return false; // All unsequenced entries are after sequenced entries.
        }

        // No sequences for either side, so tie-break by ID and conjugation status
        if (lhs.id < rhs.id) {
            return true;
        } else if ((lhs.id > rhs.id) || (lhs.conjugated == rhs.conjugated)) {
            return false;
        } else {
            return !lhs.conjugated; // true implies LHS a, RHS a*
        }

    }

}