/**
 * moment_substitution_rulebook.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rulebook.h"

#include "symbol_table.h"

namespace Moment {


    bool MomentSubstitutionRulebook::CompareByOpHash::operator()(const SymbolExpression& lhs,
                                                                 const SymbolExpression& rhs) const noexcept {
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



    MomentSubstitutionRulebook::MomentSubstitutionRulebook(const SymbolTable &symbolTable)
        : symbols{symbolTable}, comparator{symbolTable} {

    }
}