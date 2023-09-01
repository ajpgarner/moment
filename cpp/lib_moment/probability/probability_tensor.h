/**
 * probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial_tensor.h"
#include "collins_gisin.h"

#include "symbolic/polynomial.h"

#include "utilities/dynamic_bitset.h"
#include "tensor/auto_storage_tensor.h"

#include <iosfwd>
#include <span>
#include <vector>

namespace Moment {

    using ProbabilityTensorIndex = AutoStorageIndex;
    using ProbabilityTensorIndexView = AutoStorageIndexView;

    namespace errors {
        class BadPTError : public std::runtime_error {
        public:
            explicit BadPTError(const std::string& what) : std::runtime_error(what) { }
        };

    };

    class ProbabilityTensor;

    using ProbabilityTensorElement = PolynomialElement;
    using ProbabilityTensorRange = TensorRange<ProbabilityTensor>;

    /**
     * Similar to the Collins-Gisin tensor, but also includes /implicit/ dependent probabilities (e.g. a1 = 1 - a0, etc.)
     */
    class ProbabilityTensor : public PolynomialTensor {
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

    private:
        std::vector<OneDimensionInfo> dimensionInfo;

    protected:
        ProbabilityTensor(const CollinsGisin& collinsGisin, const PolynomialFactory& factory,
                          TensorConstructInfo&& constructInfo,
                          TensorStorageType storage = TensorStorageType::Automatic);

    public:
        virtual ~ProbabilityTensor() noexcept = default;

        /** Deduce information about element. */
        [[nodiscard]] ElementConstructInfo element_info(ProbabilityTensorIndexView index) const;

        /**
         * Make implicit probability rules, for a (joint) probability distribution.
         * @param measurement
         */
        [[nodiscard]] std::vector<Polynomial> explicit_value_rules(const ProbabilityTensorRange& measurement,
                                                                   std::span<const double> values) const;

        /**
         * Make implicit probability rules, for a (joint) conditional probability distribution.
         * @param measurement
         * @param conditional
         */
        [[nodiscard]] std::vector<Polynomial> explicit_value_rules(const ProbabilityTensorRange& measurement,
                                                                   const ProbabilityTensorElement& condition,
                                                                   std::span<const double> values) const;


    protected:
        [[nodiscard]] ProbabilityTensorElement make_element_no_checks(IndexView index) const override;

        [[nodiscard]] std::string get_name(bool capital) const override {
            if (capital) {
                return "Probability tensor";
            } else {
                return "probability tensor";
            }
        }

    private:
        [[nodiscard]] ProbabilityTensorElement do_make_element(IndexView elementIndex,
                                                               ElementConstructInfo& eci) const;

        void make_dimension_info(const TensorConstructInfo& info);

        void calculate_implicit_symbols();

        /** Deduce information about element, and write it to output. */
        void element_info(ProbabilityTensorIndexView index, ElementConstructInfo& output) const noexcept;

    };

}