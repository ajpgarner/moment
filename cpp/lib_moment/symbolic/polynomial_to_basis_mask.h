/**
 * polynomial_to_basis_mask.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once


#include "polynomial.h"
#include "utilities/dynamic_bitset.h"

namespace Moment {

    class SymbolTable;

    /**
     * Extract basis mask(s) from a polynomial
     */
    class PolynomialToBasisMask {
    public:
        const SymbolTable& symbols;

        using MaskType = DynamicBitset<uint64_t, symbol_name_t>;

        const double zero_tolerance = 1.0;

    public:
        explicit PolynomialToBasisMask(const SymbolTable& symbols, const double zero_tolerance)
                : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        /** Get empty mask of correct size. */
        [[nodiscard]] std::pair<MaskType, MaskType> empty_mask() const;

        /** Set bits for real and imaginary basis elements of supplied polynomial. */
        void set_bits(MaskType& real_mask, MaskType& imaginary_mask,
                      const Polynomial& poly) const;

        /** Set bits for real and imaginary basis elements of supplied monomial. */
        void set_bits(MaskType& real_mask, MaskType& imaginary_mask,
                      const Monomial& mono) const;

        /** Get masks for real and imaginary basis elements of supplied polynomial */
        [[nodiscard]] inline std::pair<MaskType, MaskType>
        operator()(const Polynomial& poly) const {
            auto output = this->empty_mask();
            this->set_bits(output.first, output.second, poly);
            return output;
        }

        /** Convert bitmasks to sets. */
        [[nodiscard]] static std::pair<std::set<symbol_name_t>, std::set<symbol_name_t>>
        masks_to_sets(const MaskType& real_mask, const MaskType& imaginary_mask);
    };
}