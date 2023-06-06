/**
 * moment_substitution_rule.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rule.h"

#include "symbol_table.h"
#include "symbol_tools.h"

#include "utilities/float_utils.h"

#include <algorithm>
#include <sstream>


namespace Moment {

    MomentSubstitutionRule::MomentSubstitutionRule(const SymbolTable& table, Polynomial &&rule)
        : lhs{rule.last_id()}, rhs{std::move(rule)} {
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
        auto [prefactor, needs_conjugate] = [&]() -> std::pair<std::complex<double>, bool> {
            assert(!rhs.empty());
            // Lambda exists to avoid this being a dangling reference
            const auto& lhs_elem = rhs.back();
            // If pre-factor is 0, rule is ill-formed.
            if (approximately_zero(lhs_elem.factor)) {
                std::stringstream errSS;
                errSS << "Polynomial rule \"" << rhs << " == 0\" is ill-formed: leading element has a pre-factor of 0.";
                throw errors::invalid_moment_rule{lhs, errSS.str()};
            }

            return {std::complex<double>(-1.0, 0) / lhs_elem.factor, lhs_elem.conjugated};
        }();

        // Remove leading element, multiply out RHS, and conjugate if necessary.
        rhs.pop_back();
        if (!approximately_equal(prefactor, 1.0)) {
            rhs *= prefactor;
        }
        if (needs_conjugate) {
            rhs.conjugate_in_place(table);
        }
    }

    Polynomial MomentSubstitutionRule::as_polynomial(const PolynomialFactory& factory) const {
        if (this->is_trivial()) {
            return Polynomial::Zero();
        }

        Polynomial as_poly{this->rhs};
        factory.append(as_poly, Polynomial{Monomial{this->lhs, -1.0, false}});
        return as_poly;
    }

    bool MomentSubstitutionRule::matches(const Polynomial &combo) const noexcept {
        return std::any_of(combo.begin(), combo.end(), [this](const Monomial& expr) {
            return expr.id == this->lhs;
        });
    }


    [[nodiscard]] std::pair<size_t, Polynomial::storage_t::const_iterator>
    MomentSubstitutionRule::match_info(const Polynomial &combo) const noexcept {
        // Look for match
        auto first_match = std::find_if(combo.begin(), combo.end(), [this](const Monomial& rhsExpr) {
            return rhsExpr.id == this->lhs;
        });

        // No matches
        if (first_match == combo.end()) {
            return {0, first_match};
        }

        // Do we also match CC?
        auto next_match = first_match + 1;
        if (next_match != combo.end()) {
            if (next_match->id == this->lhs) {
                assert(!first_match->conjugated);
                assert(next_match->conjugated);
                return {2, first_match};
            }
        }

        // Just match once
        return {1, first_match};
    }

    Polynomial MomentSubstitutionRule::reduce(const PolynomialFactory& factory, const Polynomial &combo) const {

        auto [matches, hint] = this->match_info(combo);

        // No match, copy output without transformation
        if (0 == matches) {
            return combo;
        }
        assert(hint != combo.end());

        // Otherwise, apply match
        return this->reduce_with_hint(factory, combo, hint, (matches == 2));
    }

    Polynomial MomentSubstitutionRule::reduce(const PolynomialFactory &factory, const Monomial &expr) const {
        // No match, no substitution.
        if (expr.id != this->lhs) {
            return Polynomial{rhs};
        }

        // Copy RHS, with appropriate transformations
        Polynomial::storage_t output_sequence;
        this->append_transformed(expr, std::back_inserter(output_sequence));

        // Construct as combo
        return factory(std::move(output_sequence));
    }

    Monomial MomentSubstitutionRule::reduce_monomial(const SymbolTable& table,
                                                     const Monomial &expr) const {
        // No match, pass through:
        if (this->LHS() != expr.id) {
            return expr;
        }

        if constexpr (debug_mode) {
            if (!this->rhs.is_monomial()) {
                throw std::logic_error{
                        "MomentSubstitutionRule::reduce_monomial cannot be called on non-monomial rule."};
            }
        }

        // Rule is -> 0
        if (this->rhs.empty()) {
            return Monomial{0};
        }

        // Otherwise...
        auto& monoElem = *(this->rhs.begin());
        Monomial output = (expr.conjugated ?
                           Monomial{monoElem.id, expr.factor * std::conj(monoElem.factor), !monoElem.conjugated}
                                           : Monomial{monoElem.id, expr.factor * monoElem.factor, monoElem.conjugated});
        SymbolTools{table}.make_canonical(output);
        return output;
    }

    Polynomial MomentSubstitutionRule::reduce_with_hint(const PolynomialFactory& factory,
                                                        const Polynomial &combo,
                                                        Polynomial::storage_t::const_iterator inject_iter,
                                                        const bool twice) const {
        // Hint must be good:
        assert(inject_iter != combo.end());
        assert(inject_iter + (twice ? 1 : 0) != combo.end());
        assert(inject_iter->id == this->lhs);
        assert((inject_iter + (twice ? 1 : 0))->id == this->lhs);

        // Start of LHS string
        Polynomial::storage_t output_sequence;
        std::copy(combo.begin(), inject_iter, std::back_inserter(output_sequence));

        // Copy RHS, with transformations
        this->append_transformed(*inject_iter, std::back_inserter(output_sequence));

        // Do we also have to transform complex conjugate?
        if (twice) {
            assert(inject_iter+1 != combo.end());
            this->append_transformed(*(inject_iter+1), std::back_inserter(output_sequence));
        }

        // Rest of LHS string
        std::copy(inject_iter + (twice ? 2 : 1), combo.end(), std::back_inserter(output_sequence));

        // Construct as combo
        return factory(std::move(output_sequence));

    }
}