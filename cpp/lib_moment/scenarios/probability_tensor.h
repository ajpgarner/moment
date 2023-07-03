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

#include <span>
#include <vector>

namespace Moment {

    using ProbabilityTensorIndex = std::vector<size_t>;
    using ProbabilityTensorIndexView = std::span<const size_t>;

    namespace errors {
        class BadPTError : public std::runtime_error {
        public:
            explicit BadPTError(const std::string& what) : std::runtime_error(what) { }
        };
    };

    /**
     * Similar to the Collins-Gisin tensor, but also includes /implicit/ dependent probabilities (e.g. a1 = 1 - a0, etc.)
     */
    class ProbabilityTensor {
    public:
        struct ConstructInfo {
            /** Total number of outcomes per party over all measurements. */
            std::vector<size_t> totalDimensions;

            /** Number of measurements that could be performed by each party, including trivial identity measurement.*/
            std::vector<size_t> mmtsPerParty;

            /** Number of outcomes each measurement has */
            std::vector<size_t> outcomesPerMeasurement;
        };

        /**
         * Information required to construct an element.
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

        /** The size of each dimension of the Collins Gisin (i.e. operators per party + 1) */
        const std::vector<size_t> Dimensions;

        const size_t total_size;

    private:
        std::vector<OneDimensionInfo> dimensionInfo;

        std::vector<Polynomial> data;
        DynamicBitset<uint64_t, size_t> hasSymbols;


    public:
        ProbabilityTensor(const CollinsGisin& collinsGisin, ConstructInfo&& constructInfo);

        /** Deduce information about element. */
        [[nodiscard]] ElementConstructInfo element_info(ProbabilityTensorIndexView index) const;

        /**
         * Check index is valid.
         * @throws errors::BadPTError if not valid.
         */
        void validate_indices(ProbabilityTensorIndexView index) const;

        /**
         * Convert index to offset.
         */
        [[nodiscard]] size_t index_to_offset(const ProbabilityTensorIndexView indices) const {
            this->validate_indices(indices);
            return this->index_to_offset_no_checks(indices);
        }

        [[nodiscard]] const std::vector<Polynomial>& CGPolynomials() const;




    private:
        void make_dimension_info(const ConstructInfo& info);

        void calculate_implicit_symbols();

        /** Deduce information about element, and write it to output. */
        void element_info(ProbabilityTensorIndexView index, ElementConstructInfo& output) const noexcept;

        /**
         * Convert index to offset.
         */
        [[nodiscard]] size_t index_to_offset_no_checks(CollinsGisinIndexView index) const noexcept;

    };

}