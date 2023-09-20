/**
 * polynomial_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "collins_gisin.h"

#include "symbolic/polynomial.h"
#include "tensor/auto_storage_tensor.h"
#include "utilities/dynamic_bitset.h"

#include <iosfwd>
#include <span>
#include <vector>


namespace Moment {
    struct PolynomialElement {
    public:
        Polynomial cgPolynomial;
        Polynomial symbolPolynomial;
        bool hasSymbolPoly;

    public:
        explicit PolynomialElement(Polynomial&& cgPoly)
                : cgPolynomial(std::move(cgPoly)), hasSymbolPoly{false} { }
        explicit PolynomialElement(Polynomial&& cgPoly, Polynomial&& symPoly)
                : cgPolynomial(std::move(cgPoly)), symbolPolynomial{std::move(symPoly)}, hasSymbolPoly{true} { }
    };


    /** The number of elements, below which we cache the probability tensor explicitly. */
    constexpr static const size_t poly_tensor_explicit_element_limit = 1024ULL;

    /**
     * Similar to the Collins-Gisin tensor, but also includes /implicit/ dependent probabilities (e.g. a1 = 1 - a0, etc.)
     */
    class PolynomialTensor : public AutoStorageTensor<PolynomialElement, poly_tensor_explicit_element_limit> {
    public:

    public:
        const CollinsGisin& collinsGisin;
        const PolynomialFactory& symbolPolynomialFactory;

    protected:
        /** If in explicit mode, store whether we have symbols */
        std::optional<DynamicBitset<uint64_t, size_t>> missingSymbols;
        bool hasAllSymbols = false;


    protected:
        PolynomialTensor(const CollinsGisin& collinsGisin, const PolynomialFactory& factory,
                         AutoStorageIndex&& dimensions, TensorStorageType storage = TensorStorageType::Automatic);

    public:
        virtual ~PolynomialTensor() noexcept = default;

        [[nodiscard]] Polynomial CGPolynomial(AutoStorageIndexView index) const;

        /** True if all polynomials have been filled (or tensor is virtual). */
        [[nodiscard]] bool HasAllPolynomials() const noexcept {
            return this->hasAllSymbols;
        }

        /** Attempts to fill missing polynomials */
        bool fill_missing_polynomials();


        /**
         * Get string representation of polynomial.
         */
        [[nodiscard]] std::string elem_as_string(const PolynomialElement& element) const;

        /**
         * Write string representation of polynomial to stream.
         */
        void elem_as_string(std::ostream& os, const PolynomialElement& element) const;

    protected:
        /** Try to get actual polynomial values for element, if they exist. */
        bool attempt_symbol_resolution(PolynomialElement& element) const;

    };

}