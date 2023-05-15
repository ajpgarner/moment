/**
 * compare_by_op_hash.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "compare_by_op_hash.h"

#include "symbol_table.h"

namespace Moment {

    bool CompareByOpHash::operator()(const SymbolExpression& lhs, const SymbolExpression& rhs) const noexcept {
        assert(lhs.id < this->symbolTable.size());
        assert(rhs.id < this->symbolTable.size());
        const auto lhs_hash = this->symbolTable[lhs.id].hash();
        const auto rhs_hash = this->symbolTable[rhs.id].hash();

        if (lhs_hash < rhs_hash) {
            return true;
        } else if ((lhs_hash > rhs_hash) || (lhs.conjugated == rhs.conjugated)) {
            return false;
        }

        return !lhs.conjugated; // true implies lhs a, rhs a*
    }
}