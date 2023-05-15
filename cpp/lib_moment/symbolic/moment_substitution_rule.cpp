/**
 * moment_substitution_rule.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rule.h"

#include <algorithm>

namespace Moment {

    bool MomentSubstitutionRule::matches(const SymbolCombo &combo) const noexcept {
        return std::any_of(combo.begin(), combo.end(), [this](const SymbolExpression& expr) {
            return expr.id == this->lhs;
        });
    }

    SymbolCombo MomentSubstitutionRule::reduce(const SymbolCombo &combo) const {
        // Find matching symbol
        auto inject_iter = std::find_if(combo.begin(), combo.end(), [this](const SymbolExpression& rhsExpr) {
            return rhsExpr.id == this->lhs;
        });

        // No match, copy output
        if (inject_iter == combo.end()) {
            return combo;
        }

        // Match is first symbol
        SymbolCombo::storage_t output_sequence;

        // Start of LHS string
        std::copy(combo.begin(), inject_iter, std::back_inserter(output_sequence));

        // Copy RHS, with transformations
        const auto& matchExpr = *inject_iter;
        std::transform(rhs.begin(), rhs.end(), std::back_inserter(output_sequence),
                       [&matchExpr](SymbolExpression src) {
            if (matchExpr.conjugated) {
                src.conjugated = !src.conjugated;
            }
            src.factor *= matchExpr.factor;
            return src;
        });

        // Rest of LHS string
        std::copy(inject_iter + 1, combo.end(), std::back_inserter(output_sequence));

        // Construct as combo
        return SymbolCombo(std::move(output_sequence), *table);
    }
}