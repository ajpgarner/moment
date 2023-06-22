/**
 * moment_rulebook_to_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_rulebook_to_basis.h"

#include "moment_rulebook.h"

#include "../polynomial_factory.h"
#include "../polynomial_to_basis.h"
#include "../polynomial_to_basis_mask.h"

#include "../symbol_table.h"

namespace Moment {

    MomentRulebookToBasis::MomentRulebookToBasis(const PolynomialFactory& factory,
                                                 MomentRulebookToBasis::ExportMode mode)
         : symbols{factory.symbols}, zero_tolerance{factory.zero_tolerance}, export_mode{mode} {
    }

    MomentRulebookToBasis::MomentRulebookToBasis(const SymbolTable& symbols, double zero_tolerance,
                                                 MomentRulebookToBasis::ExportMode mode)
         : symbols{symbols}, zero_tolerance{zero_tolerance}, export_mode{mode} {
    }

    MomentRulebookToBasis::output_t MomentRulebookToBasis::operator()(const MomentRulebook& rulebook) const {
        assert(&rulebook.symbols == &symbols);
        using Index = MomentRulebookToBasis::output_t::Index;

        const auto num_real_elems = static_cast<Index>(this->symbols.Basis.RealSymbolCount());
        const auto num_im_elems = static_cast<Index>(this->symbols.Basis.ImaginarySymbolCount());
        const auto num_elems = static_cast<Index>(num_real_elems + num_im_elems);

        // Record which basis elements are constrained by rules...
        PolynomialToBasisMask ptbm{this->symbols, this->zero_tolerance};
        auto [mask_real, mask_imaginary] = ptbm.empty_mask();

        PolynomialToBasisVec to_basis{this->symbols, this->zero_tolerance};
        std::vector<Eigen::Triplet<double>> triplets;

        for (const auto& [symbol_id, rule] : rulebook) {
            assert(symbol_id < this->symbols.size());
            const auto& lhs_info = this->symbols[symbol_id];
            const auto [lhs_re_index, lhs_im_index] = lhs_info.basis_key();
            const bool lhs_has_real = lhs_re_index >= 0;
            const bool lhs_has_im = lhs_im_index >= 0;

            if (rule.is_partial()) {
                if (approximately_real(rule.partial_direction(), this->zero_tolerance)) {
                    to_basis.add_triplet_row(rule.RHS(), lhs_re_index, -1, triplets);
                    mask_real.set(lhs_re_index);
                } else if (approximately_imaginary(rule.partial_direction(), this->zero_tolerance)) {
                    to_basis.add_triplet_row(rule.RHS(), -1, lhs_im_index, triplets);
                    mask_imaginary.set(lhs_im_index);
                } else {
                    assert(lhs_has_real);
                    assert(lhs_has_im);

                    // Real and imaginary bits!?
                    const double cos_delta = rule.partial_direction().real();
                    const double sin_delta = rule.partial_direction().imag();

                    // We will leave one of a or b unconstrained, in a manner to ensure numeric stability;
                    // Particularly, 1/cos(d) and 1/sin(d) range from 1 to sqrt(2), and tan(d)/cot(d) range from 0 to 1.
                    const bool mostly_real = abs(cos_delta) >= abs(sin_delta);

                    // Copy rule RHS, removing last two terms, then rotate to real part
                    auto rule_rhs{rule.RHS()};
                    rule_rhs.pop_back();
                    rule_rhs.pop_back();
                    rule_rhs *= std::conj(rule.partial_direction());

                    // We will constrain a, and leave b mostly unconstrained
                    if (mostly_real) {
                        rule_rhs *= 1.0 / cos_delta;
                        to_basis.add_triplet_row(rule_rhs, lhs_re_index, -1, triplets); // ignores Im part
                        triplets.emplace_back(lhs_re_index, num_real_elems + lhs_im_index, -sin_delta/cos_delta);
                        mask_real.set(lhs_re_index);
                    } else { // We will constrain b, and leave a mostly unconstrained
                        rule_rhs *= 1.0 / sin_delta;
                        to_basis.add_triplet_row(rule_rhs, num_real_elems + lhs_im_index, -1, triplets); // ignores Im part
                        triplets.emplace_back(num_real_elems + lhs_im_index, lhs_re_index, -cos_delta/sin_delta);
                        mask_imaginary.set(lhs_im_index);
                    }
                }
            } else {
                // Add real and imaginary parts of symbol to triplets
                to_basis.add_triplet_row(rule.RHS(), lhs_re_index, lhs_im_index, triplets);

                // Flag as written
                if (lhs_has_real) {
                    mask_real.set(lhs_re_index);
                }
                if (lhs_has_im) {
                    mask_imaginary.set(lhs_im_index);
                }
            }
        }

        if (export_mode == ExportMode::Rewrite) {
            // Insert ID for non-mentioned elements
            mask_real.invert_in_place();
            for (auto index: mask_real) {
                triplets.emplace_back(index, index, 1.0);
            }
            mask_imaginary.invert_in_place();
            for (auto index: mask_imaginary) {
                triplets.emplace_back(num_real_elems + index, num_real_elems + index, 1.0);
            }
        } else {
            assert(export_mode == ExportMode::Homogeneous);
            // Subtract ID from mentioned elements
            for (auto index: mask_real) {
                triplets.emplace_back(index, index, -1.0);
            }

            for (auto index: mask_imaginary) {
                triplets.emplace_back(num_real_elems + index, num_real_elems + index, -1.0);
            }
        }

        // Construct matrix
        output_t output{num_elems, num_elems};
        output.setFromTriplets(triplets.begin(), triplets.end());
        return output;
    }

}