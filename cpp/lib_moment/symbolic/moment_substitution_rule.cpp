/**
 * moment_substitution_rule.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rule.h"

#include "utilities/float_utils.h"

#include <algorithm>
#include <sstream>


namespace Moment {

    MomentSubstitutionRule::MomentSubstitutionRule(const SymbolTable &symbols, SymbolCombo &&rule)
        : table{&symbols}, lhs{rule.last_id()}, rhs{std::move(rule)} {
        // Trivial rule?
        if (0 == lhs) {
            rhs.clear();
            return;
        }

        // If LHS of rule is a scalar, rule is ill-formed.
        if (1 == this->lhs) {
            std::stringstream errSS;
            errSS << "Polynomial rule \"" << rhs << " == 0\" is ill-formed: it implies a scalar value is zero.";
            throw errors::invalid_moment_rule{lhs, errSS.str()};
        }

        // Extract information about last element
        auto [prefactor, needs_conjugate] = [&]() -> std::pair<double, bool> {
            assert(!rhs.empty());
            // Lambda exists to avoid this being a dangling reference
            const auto& lhs_elem = rhs.back();
            // If pre-factor is 0, rule is ill-formed.
            if (approximately_zero(lhs_elem.factor)) {
                std::stringstream errSS;
                errSS << "Polynomial rule \"" << rhs << " == 0\" is ill-formed: leading element has a pre-factor of 0.";
                throw errors::invalid_moment_rule{lhs, errSS.str()};
            }
            return {-1.0 / lhs_elem.factor, lhs_elem.conjugated};
        }();

        // Remove leading element, multiply out RHS, and conjugate if necessary.
        rhs.pop_back();
        if (!approximately_equal(prefactor, 1.0)) {
            rhs *= prefactor;
        }
        if (needs_conjugate) {
            rhs.conjugate_in_place(*table);
        }
    }

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