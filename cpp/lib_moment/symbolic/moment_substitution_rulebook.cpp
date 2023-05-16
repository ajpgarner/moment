/**
 * moment_substitution_rulebook.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rulebook.h"

#include "symbol_table.h"

#include <algorithm>
#include <stdexcept>

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

    void MomentSubstitutionRulebook::add_raw_rules(std::vector<SymbolCombo> &&raw) {
        // Cannot add rules after completion.
        assert(this->rules.empty());

        // Move in rules
        if (this->raw_rules.empty()) {
            this->raw_rules = std::move(raw);
        } else {
            this->raw_rules.reserve(this->raw_rules.size() + raw.size());
            std::move(raw.begin(), raw.end(), std::back_inserter(this->raw_rules));
        }
    }

    bool MomentSubstitutionRulebook::complete() {
        // Rules already complete:
        if (!this->rules.empty()) {
            return true;
        }

        // Nothing to do, thus, already complete
        if (this->raw_rules.empty()) {
            return true;
        }

        // First, sort raw rules by lowest leading monomial, tie-breaking with shorter strings first.
        std::sort(this->raw_rules.begin(), this->raw_rules.end(), [](const auto& lhs, const auto& rhs) {
            const auto lhs_id = lhs.last_id();
            const auto rhs_id = rhs.last_id();
            if (lhs_id < rhs_id) {
                return true;
            } else if (lhs_id > rhs_id) {
                return false;
            }
            return lhs.size() < rhs.size();
        });

        throw std::logic_error{"MomentSubstitutionRulebook::complete() not implemented"};

        // Rules now complete
        return true;
    }

}