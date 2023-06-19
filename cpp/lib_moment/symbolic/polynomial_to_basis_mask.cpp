/**
 * polynomial_to_basis_mask.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_to_basis_mask.h"

#include "symbolic/symbol_table.h"

namespace Moment {
    std::pair<PolynomialToBasisMask::MaskType, PolynomialToBasisMask::MaskType>
    PolynomialToBasisMask::empty_mask() const {
        return {MaskType(static_cast<symbol_name_t>(this->symbols.Basis.RealSymbolCount())),
                MaskType(static_cast<symbol_name_t>(this->symbols.Basis.ImaginarySymbolCount()))};
    }

    void PolynomialToBasisMask::set_bits(PolynomialToBasisMask::MaskType& real_mask,
                                         PolynomialToBasisMask::MaskType& imaginary_mask,
                                         const Polynomial &poly) const {
        assert(real_mask.bit_size == this->symbols.Basis.RealSymbolCount());
        assert(imaginary_mask.bit_size == this->symbols.Basis.ImaginarySymbolCount());

        auto poly_iter = poly.begin();
        while (poly_iter != poly.end()) {
            auto& monomial = *poly_iter;
            assert(monomial.id < symbols.size());
            auto& symbol_info = this->symbols[monomial.id];
            auto [re_basis_index, im_basis_index] = symbol_info.basis_key();

            // Purely imaginary symbol:
            if (re_basis_index < 0) {
                if (im_basis_index >= 0) {
                    imaginary_mask.set(im_basis_index);
                } // else, zero.
                ++poly_iter;
                continue;
            }

            // Purely real symbol
            if (im_basis_index < 0) {
                assert(re_basis_index >= 0);
                real_mask.set(re_basis_index);
                ++poly_iter;
                continue;
            }

            // Complex symbol: need to check if conjugate appears as next term...
            auto peek_iter = poly_iter + 1;
            if ((peek_iter != poly.end()) && (peek_iter->id == poly_iter->id)) {
                assert(poly_iter->conjugated != peek_iter->conjugated);
                if (approximately_same_norm(peek_iter->factor, poly_iter->factor, this->zero_tolerance)) {

                    if (!(approximately_zero(peek_iter->factor, this->zero_tolerance))) {
                        auto ratio = poly_iter->factor / peek_iter->factor;
                        if (!approximately_real(ratio, this->zero_tolerance)) {
                            // Mono-dimensional value, but not aligned with real or imaginary axis. Include both:
                            real_mask.set(re_basis_index);
                            imaginary_mask.set(im_basis_index);
                        } else {
                            if (ratio.real() >= 0) { // X + X*; real only
                                real_mask.set(re_basis_index);
                            } else { // X - X*; imaginary only
                                imaginary_mask.set(im_basis_index);
                            }
                        }

                    } // Both symbols are close to zero, so don't add either...
                } else {
                    // Both symbol and conjugate appear, but they don't cancel out.
                    real_mask.set(re_basis_index);
                    imaginary_mask.set(im_basis_index);
                }

                // Skip symbol's conjugate
                ++poly_iter;
                ++poly_iter;
                continue;
            }

            // Complex symbol, but conjugate doesn't appear - include both terms.
            real_mask.set(re_basis_index);
            imaginary_mask.set(im_basis_index);

            ++poly_iter;
        }
    }


    void PolynomialToBasisMask::set_bits(PolynomialToBasisMask::MaskType& real_mask,
                                         PolynomialToBasisMask::MaskType& imaginary_mask,
                                         const Monomial& monomial) const {
        assert(real_mask.bit_size == this->symbols.Basis.RealSymbolCount());
        assert(imaginary_mask.bit_size == this->symbols.Basis.ImaginarySymbolCount());
        assert(monomial.id < symbols.size());
        if ((monomial.id == 0) || approximately_zero(monomial.factor, this->zero_tolerance)) {
            return;
        }
        auto& symbol_info = this->symbols[monomial.id];
        auto [re_basis_index, im_basis_index] = symbol_info.basis_key();
        if (re_basis_index > 0) {
            real_mask.set(re_basis_index);
        }
        if (im_basis_index > 0) {
            imaginary_mask.set(im_basis_index);
        }
    }

    std::pair<std::set<symbol_name_t>, std::set<symbol_name_t>>
    PolynomialToBasisMask::masks_to_sets(const PolynomialToBasisMask::MaskType& real_mask,
                                         const PolynomialToBasisMask::MaskType& imaginary_mask) {
        return std::make_pair(real_mask.to_set(), imaginary_mask.to_set());
    }
}