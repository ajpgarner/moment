/**
 * probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "collins_gisin.h"

#include "symbolic/polynomial.h"

#include "utilities/dynamic_bitset.h"
#include "utilities/tensor.h"

#include <iosfwd>
#include <span>
#include <vector>

namespace Moment {

    using ProbabilityTensorIndex = Tensor::Index;
    using ProbabilityTensorIndexView = Tensor::IndexView;

    namespace errors {
        class BadPTError : public std::runtime_error {
        public:
            explicit BadPTError(const std::string& what) : std::runtime_error(what) { }
        };

    };

    /** The number of elements, below which we cache the probability tensor explicitly. */
    constexpr static const size_t PT_explicit_element_limit = 1024ULL;

    struct ProbabilityTensorElement {
    public:
        Polynomial cgPolynomial;
        Polynomial symbolPolynomial;
        bool hasSymbolPoly;

    public:
        explicit ProbabilityTensorElement(Polynomial&& cgPoly)
            : cgPolynomial(std::move(cgPoly)), hasSymbolPoly{false} { }
        explicit ProbabilityTensorElement(Polynomial&& cgPoly, Polynomial&& symPoly)
            : cgPolynomial(std::move(cgPoly)), symbolPolynomial{std::move(symPoly)}, hasSymbolPoly{true} { }
    };

    class ProbabilityTensor;

    using ProbabilityTensorRange = TensorRange<ProbabilityTensor>;

    /**
     * Similar to the Collins-Gisin tensor, but also includes /implicit/ dependent probabilities (e.g. a1 = 1 - a0, etc.)
     */
    class ProbabilityTensor : public AutoStorageTensor<ProbabilityTensorElement, PT_explicit_element_limit> {
    public:
        /** Utility structure: grouping of properties required to set up tensor. */
        struct TensorConstructInfo {
            /** Total number of outcomes per party over all measurements. */
            std::vector<size_t> totalDimensions;

            /** Number of measurements that could be performed by each party, including trivial identity measurement.*/
            std::vector<size_t> mmtsPerParty;

            /** Number of outcomes each measurement has */
            std::vector<size_t> outcomesPerMeasurement;

            /** True, if measurement does not need an implicit symbol. */
            std::vector<bool> fullyExplicit;
        };

        /**
         * Information required to calculate a single element.
         */
        struct ElementConstructInfo {
            CollinsGisinIndex baseIndex;
            CollinsGisinIndex finalIndex;
            std::vector<size_t> implicitMmts;

        public:
            explicit ElementConstructInfo(size_t dimensions) : baseIndex(dimensions, 0), finalIndex(dimensions, 0) {
                implicitMmts.reserve(dimensions);
            }
        };

    private:
        /** Information about each tensor axis. */
        struct OneDimensionInfo {
            /** Global measurement ID */
            std::vector<size_t> measurement;
            /** Corresponding index in CG tensor, or index to first element of measurement if implicit. */
            std::vector<size_t> cgDimensionIndex;
            /** Measurement outcome number. */
            std::vector<oper_name_t> outcome_index;
            /** Is this element implicit */
            DynamicBitset<uint64_t, size_t> implicit;

        public:
            explicit OneDimensionInfo(size_t size) : implicit{size} {
                this->measurement.reserve(size);
                this->cgDimensionIndex.reserve(size);
                this->outcome_index.reserve(size);
            }

            [[nodiscard]] bool is_implicit(const size_t elem_index) const noexcept {
                return this->implicit.test(elem_index);
            }
        };

    public:
        const CollinsGisin& collinsGisin;
        const PolynomialFactory& symbolPolynomialFactory;

    private:
        std::vector<OneDimensionInfo> dimensionInfo;

        /** If in explicit mode, store whether we have symbols */
        DynamicBitset<uint64_t, size_t> missingSymbols;
        bool hasAllSymbols = false;


    protected:
        ProbabilityTensor(const CollinsGisin& collinsGisin, const PolynomialFactory& factory,
                          TensorConstructInfo&& constructInfo,
                          TensorStorageType storage = TensorStorageType::Automatic);

    public:
        virtual ~ProbabilityTensor() noexcept = default;

        /** Deduce information about element. */
        [[nodiscard]] ElementConstructInfo element_info(ProbabilityTensorIndexView index) const;

        [[nodiscard]] Polynomial CGPolynomial(ProbabilityTensorIndexView index) const;

        /** True if all polynomials have been filled (or tensor is virtual). */
        [[nodiscard]] bool HasAllPolynomials() const noexcept {
            return this->hasAllSymbols;
        }

        /** Attempts to fill missing polynomials */
        bool fill_missing_polynomials();

        /**
         * Make implicit probability rules.
         * @param measurement
         */
        [[nodiscard]] std::vector<Polynomial> explicit_value_rules(const ProbabilityTensorRange& measurement,
                                                                   std::span<const double> values) const;

        /**
         * Make implicit probability rules.
         * @param measurement
         * @param conditional
         */
        [[nodiscard]] std::vector<Polynomial> explicit_value_rules(const ProbabilityTensorRange& measurement,
                                                                   const ProbabilityTensorElement& condition,
                                                                   std::span<const double> values) const;


        /**
         * Get string representation of polynomial.
         */
        [[nodiscard]] std::string elem_as_string(const ProbabilityTensorElement& element) const;

        void elem_as_string(std::ostream& os, const ProbabilityTensorElement& element) const;

    protected:
        [[nodiscard]] ProbabilityTensorElement make_element_no_checks(Tensor::IndexView index) const override;

        [[nodiscard]] std::string get_name(bool capital) const override {
            if (capital) {
                return "Probability tensor";
            } else {
                return "probability tensor";
            }
        }

    private:
        [[nodiscard]] ProbabilityTensorElement do_make_element(Tensor::IndexView elementIndex,
                                                               ElementConstructInfo& eci) const;

        void make_dimension_info(const TensorConstructInfo& info);

        void calculate_implicit_symbols();

        /** Deduce information about element, and write it to output. */
        void element_info(ProbabilityTensorIndexView index, ElementConstructInfo& output) const noexcept;

        /** Try to get actual polynomial values for element, if they exist. */
        bool attempt_symbol_resolution(ProbabilityTensorElement& element);

    };

}