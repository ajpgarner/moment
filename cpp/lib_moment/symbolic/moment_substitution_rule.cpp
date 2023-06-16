/**
 * moment_substitution_rule.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rule.h"

#include "polynomial_factory.h"
#include "symbol_table.h"
#include "symbol_tools.h"

#include "utilities/float_utils.h"

#include <algorithm>
#include <sstream>


namespace Moment {

    namespace {
#ifndef NDEBUG
        inline bool DEBUG_assert_nonorientable(const Polynomial &poly, double mult) {
            if (poly.size() < 2) {
                return false;
            }
            const auto &leading = poly[poly.size() - 1];
            const auto &second = poly[poly.size() - 2];
            if (leading.id != second.id) {
                return false;
            }
            if (!leading.conjugated || second.conjugated) {
                return false;
            }
            if (!approximately_same_norm(leading.factor, second.factor, mult)) {
                return false;
            }
            return true;
        }
#endif

        symbol_name_t pop_back_and_normalize(const PolynomialFactory& factory, Polynomial& poly) {
            // Empty polynomial is already normalized
            if (poly.empty()) {
                return 0;
            }

            // Extract information about last element
            auto [symbol_id, prefactor, needs_conjugate] = [&]() -> std::tuple<symbol_name_t, std::complex<double>, bool> {
                // Lambda exists to avoid this being a dangling reference
                assert(!poly.empty());
                const auto& lhs_elem = poly.back();

                return {lhs_elem.id, std::complex<double>(-1.0, 0) / lhs_elem.factor, lhs_elem.conjugated};
            }();

            // Remove leading element, multiply out RHS, and conjugate if necessary.
            poly.pop_back();
            if (!approximately_equal(prefactor, 1.0)) {
                poly *= prefactor;
            }
            if (needs_conjugate) {
                poly.conjugate_in_place(factory.symbols);
            }
            poly.real_or_imaginary_if_close(factory.zero_tolerance);
            return symbol_id;
        }
    }

    MomentSubstitutionRule::MomentSubstitutionRule(const PolynomialFactory& factory,
                                                   symbol_name_t lhs, std::complex<double> constraint_direction,
                                                   Polynomial&& replacement)
       : lhs{lhs}, rhs{std::move(replacement)}, partial{true}, lhs_direction{constraint_direction} {
        assert(rhs.is_hermitian(factory.symbols, factory.zero_tolerance));
        assert(rhs.last_id() < lhs);

        // Substitution rule will then be of form X -> e^id P + 0.5X - 0.5 e^2id X*
        real_or_imaginary_if_close(this->lhs_direction, factory.zero_tolerance);
        this->rhs *=this->lhs_direction;
        factory.append(this->rhs,
                       factory({Monomial{this->lhs, 0.5, false},
                                Monomial{this->lhs, -0.5 * this->lhs_direction * this->lhs_direction, true}}));

        this->rhs.real_or_imaginary_if_close(factory.zero_tolerance);
    }


    MomentSubstitutionRule::MomentSubstitutionRule(const PolynomialFactory& factory, Polynomial &&rule)
            : lhs{rule.last_id()}, rhs{std::move(rule)} {
        auto difficulty = MomentSubstitutionRule::get_difficulty(rhs, factory.zero_tolerance);
        this->set_up_rule(factory, difficulty);
    }

    MomentSubstitutionRule::MomentSubstitutionRule(const PolynomialFactory& factory,
                                                   Polynomial &&rule,
                                                   const PolynomialDifficulty difficulty)
            : lhs{rule.last_id()}, rhs{std::move(rule)} {
        this->set_up_rule(factory, difficulty);
    }

    void MomentSubstitutionRule::set_up_rule(const PolynomialFactory& factory, PolynomialDifficulty difficulty) {
        switch(difficulty) {
            case PolynomialDifficulty::Trivial:
                rhs.clear();
                return;
            case PolynomialDifficulty::Contradiction: {
                std::stringstream errSS;
                errSS << "Polynomial rule \"" << rhs << " == 0\" is ill-formed: it implies a scalar value is zero.";
                throw errors::invalid_moment_rule{lhs, errSS.str()};
            }
            case PolynomialDifficulty::Simple:
                pop_back_and_normalize(factory, this->rhs);
                this->split_regular_rule(factory);
                break;
            case PolynomialDifficulty::NeedsReorienting:
                this->rhs = MomentSubstitutionRule::reorient_polynomial(factory, std::move(this->rhs));
                pop_back_and_normalize(factory, this->rhs);
                this->split_regular_rule(factory);
                break;
            case PolynomialDifficulty::NonorientableRule:
                this->resolve_nonorientable_rule(factory);
                break;
            case PolynomialDifficulty::Unknown:
            default:
                throw std::runtime_error{
                    "Cannot initialize a MomentSubstitutionRule without first testing polynomial."
                };
        }
    }

    MomentSubstitutionRule::PolynomialDifficulty
    MomentSubstitutionRule::get_difficulty(const Polynomial &poly, const double tolerance) noexcept {
        // Is rule of form 0 == 0 ?
        if (poly.empty()) {
            return PolynomialDifficulty::Trivial;
        }

        // Is rule of form 1 == 0?
        if (1 == poly.last_id()) {
            return PolynomialDifficulty::Contradiction;
        }

        // Is rule of form <X> == 0
        if (poly.size() <= 1) {
            return PolynomialDifficulty::Simple;
        }

        // Is rule of form <X> -> P, where P contains no terms in X*.
        const auto& leading_elem = poly[poly.size()-1];
        const auto& second_leading_elem = poly[poly.size()-2];
        if (leading_elem.id != second_leading_elem.id) {
            return PolynomialDifficulty::Simple;
        }

        // Can we re-arrange rule to the form <X> -> P, where P contains no terms in X*?
        if (!approximately_same_norm(leading_elem.factor, second_leading_elem.factor, tolerance)) {
            return PolynomialDifficulty::NeedsReorienting;
        }

        // If not, we have a meaningful, but difficult rule
        return PolynomialDifficulty::NonorientableRule;
    }



    Polynomial MomentSubstitutionRule::reorient_polynomial(const PolynomialFactory& factory, Polynomial rule) {
        auto conjugate_rule = rule.conjugate(factory.symbols);

        [[maybe_unused]] auto fwd_leading_id = pop_back_and_normalize(factory, rule);
        [[maybe_unused]] auto rev_leading_id = pop_back_and_normalize(factory, conjugate_rule);
        assert(fwd_leading_id == rev_leading_id);

        factory.append(rule, conjugate_rule * -1.0);
        return rule;
    }

    void MomentSubstitutionRule::resolve_nonorientable_rule(const PolynomialFactory &factory) {
        // Essentially, we need to identify the constrained direction e^id, where e^id is in the upper half-plane.
        // Thus, we  take X and X* to the LHS and rotate such that the LHS is: Kd(X) := 0.5 e^-id X + 0.5 e^id X*.
        // As Kd(X) is real, we take only the real part of (rotated) RHS, and split off the imaginary remainder.
        //  We then rotate the rule back to point in the direction of Kd(X), and add the unconstrained part.
        // The unconstrained part is given by Jd(X) := -0.5 i e^-id X + 0.5 i e^id X*.
        // Note, e^-id X = Kd(X) + iJd(X); and so X = e^id Kd(X) + ie^id Jd(X)
        // leading to final form of the rule: X -> e^id Kd(X) + 0.5 X - 0.5 e^2id X*

        assert(DEBUG_assert_nonorientable(this->rhs, factory.zero_tolerance));
        this->partial = true;
        this->lhs = this->rhs.last_id();

        // Initially, polynomial is:  k exp{ia} X + k exp{ib} X* + P = 0;
        const auto k_exp_i_a = this->rhs[this->rhs.size()-2].factor;
        const auto exp_i_b_minus_a = this->rhs[this->rhs.size()-1].factor / k_exp_i_a;
        assert(approximately_equal(std::norm(exp_i_b_minus_a), 1.0, factory.zero_tolerance));

        // To get e{id} := exp{i(b-a)/2}, we take the square root of exp{i(b-a)}
        // std::sqrt is in right half plane, but we want e^id in the upper half plane, including +1, excluding -1.

        if (approximately_real(exp_i_b_minus_a, factory.zero_tolerance)) {
            // Handle real case separately 1. for speed, and 2. to avoid errors with -0.0.
            if (exp_i_b_minus_a.real() > 0) {
                this->lhs_direction = 1.0;
            } else {
                this->lhs_direction = std::complex{0.0, 1.0};
            }
        } else {
            this->lhs_direction = (exp_i_b_minus_a.imag() >= 0) ? std::sqrt(exp_i_b_minus_a)
                                                                : -std::sqrt(exp_i_b_minus_a);
        }


        // Now we can safely remove terms in X and X* from RHS polynomial.
        this->rhs.pop_back();
        this->rhs.pop_back();

        // We need to rotate and scale the RHS: k exp{ia} X + k exp{ib} X* = -P;
        // multiply by 0.5 exp{-i(a+b)/2} k^-1:
        //          0.5 exp{i{a-b}/2} X + 0.5 exp{i{b-a}/2} X* = -0.5 k^-1 exp{-i(a+b)/2) P
        // Using  e{-i(b-a)/2} * e^-ia = exp{-i(b+a)/2}; so we make this transformation:
        const auto poly_factor = -std::conj(this->lhs_direction) / (2.0 * k_exp_i_a);
        this->rhs *= poly_factor;

        // Rule now has real LHS. Thus, we split off imaginary part of RHS (if any), and ensure RHS is purely real.
        this->split_polynomial = this->rhs.Imaginary(factory);
        if (this->split_polynomial->empty()) {
            this->split_polynomial.reset();
        } else {
            this->rhs = this->rhs.Real(factory);
        }

        // Finally, rotate rule back by e^id and insert non-constrained part of X [i e^id Jd(X)]
        this->rhs *= this->lhs_direction;
        const auto factor_x_star = -this->lhs_direction * this->lhs_direction * 0.5;
        factory.append(this->rhs, Polynomial{{Monomial{this->lhs, 0.5, false},
                                              Monomial{this->lhs, factor_x_star, true}}});

        // Clean values.
        this->rhs.real_or_imaginary_if_close(factory.zero_tolerance);
        real_or_imaginary_if_close(this->lhs_direction);

    }


    void MomentSubstitutionRule::merge_partial(const PolynomialFactory& factory, MomentSubstitutionRule&& other) {
        // Assert compatibility
        assert(this->partial);
        assert(other.partial);
        assert(this->lhs == other.lhs);
        assert(this->rhs.size() >= 2);


        // Same direction component should have always been projected out!
        const std::complex<double> relative_angle = other.lhs_direction / this->lhs_direction;
        assert(approximately_imaginary(relative_angle, factory.zero_tolerance));

        // Remove terms in X from this and other RHS
        this->rhs.pop_back();
        this->rhs.pop_back();
        other.rhs.pop_back();
        other.rhs.pop_back();
        if (relative_angle.imag() > 0) { // other = i this
            factory.append(this->rhs, other.rhs);
        } else { // other = - i this
            other.rhs *= -1.0;
            factory.append(this->rhs, other.rhs);
        }

        // Rule is now full.
        this->partial = false;
        this->lhs_direction = 0.0;
    }

    std::optional<Polynomial> MomentSubstitutionRule::split() {
        if (!this->split_polynomial.has_value()) {
            return std::nullopt;
        }

        // Release output
        std::optional<Polynomial> output = std::move(this->split_polynomial);
            this->split_polynomial.reset();
        return output;
    }

    void MomentSubstitutionRule::split_regular_rule(const PolynomialFactory &factory) {

        // Do nothing for trivial (or contradictory!) rules.
        if (this->lhs <= 1) {
            return;
        }

        // Split regular rule
        assert(this->lhs < factory.symbols.size());
        const auto& symbolInfo = factory.symbols[this->lhs];
        if (symbolInfo.is_hermitian()) {
            // If LHS and RHS is Hermitian, then taking Im(LHS) == Im(RHS) gives trivially 0 == 0.
            if (factory.is_hermitian(this->rhs)) {
                return;
            }

            // We have non-trivial case where LHS is Hermitian, but RHS is not.
            Polynomial output{this->rhs.Imaginary(factory)}; // Imaginary part is an expression equal to zero!
            assert(!output.empty()); // We have just inferred that we don't have hermitian RHS!

            // Force realness on the RHS of the rule.
            this->rhs = this->rhs.Real(factory);

            // Return imaginary 'remainder'.
            this->split_polynomial = std::move(output);
            return;
        }

        if (symbolInfo.is_antihermitian()) {
            // If LHS and RHS are anti-Hermitian, then taking Re(LHS) == Re(RHS) gives trivially 0 == 0
            if (factory.is_antihermitian(this->rhs)) {
                return;
            }

            // We have non-trivial case where LHS is anti-Hermitian, but RHS is not
            Polynomial output{this->rhs.Real(factory)};
            assert(!output.empty()); // We have just inferred that RHS is not anti-Hermitian.

            // Force imaginariness on the RHS of this rule.
            this->rhs = this->rhs.Imaginary(factory) * std::complex<double>(0.0, 1.0); // LHS -> i Im(RHS)

            // Return real 'remainder'.
            this->split_polynomial = std::move(output);
            return;
        }

        // nothing happened
    }


    Polynomial MomentSubstitutionRule::as_polynomial(const PolynomialFactory& factory) const {
        if (this->is_trivial()) {
            return Polynomial::Zero();
        }

        Polynomial as_poly{this->rhs};
        factory.append(as_poly, Polynomial{Monomial{this->lhs, -1.0, false}});
        as_poly.real_or_imaginary_if_close(factory.zero_tolerance);
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
            return Polynomial{expr};
        }

        // Copy RHS, with appropriate transformations
        Polynomial::storage_t output_sequence;
        this->append_transformed(expr, std::back_inserter(output_sequence));

        // Construct as combo
        return factory(std::move(output_sequence));
    }

    Monomial MomentSubstitutionRule::reduce_monomial(const SymbolTable& table,
                                                     const Monomial &expr) const {
        if constexpr (debug_mode) {
            if (!this->rhs.is_monomial()) {
                throw std::logic_error{
                        "MomentSubstitutionRule::reduce_monomial cannot be called on non-monomial rule."};
            }
        }

        // No match, pass through:
        if (this->LHS() != expr.id) {
            return expr;
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