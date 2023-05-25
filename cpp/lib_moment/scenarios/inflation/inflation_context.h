/**
 * inflation_context.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../context.h"
#include "../operator_sequence.h"

#include "causal_network.h"
#include "observable_variant_index.h"

#include "utilities/dynamic_bitset.h"
#include "utilities/small_vector.h"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <span>
#include <vector>

namespace Moment::Inflation {

    using SourceListBitset = DynamicBitset<uint64_t, SmallVector<uint64_t, 1>>;


    class InflationContext : public Context {
    public:
        /** Extra operator information for inflation scenario */
        struct ICOperatorInfo {
            oper_name_t global_id;
            oper_name_t observable;
            oper_name_t flattenedSourceIndex;
            oper_name_t outcome;
            bool projective;

        public:
            ICOperatorInfo() = default;

            ICOperatorInfo(oper_name_t id, oper_name_t observable, oper_name_t flattenedIndex,
                           oper_name_t outcome, bool projective)
                : global_id{id}, observable{observable}, flattenedSourceIndex{flattenedIndex},
                  outcome{outcome}, projective{projective} { }

            /**
            * Predicate: true if the operator id of LHS is less than that of RHS.
            */
            struct OrderByID {
                constexpr OrderByID() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    return lhs.global_id < rhs.global_id;
                }
            };

            /**
             * Predicate: true if lhs != rhs, but both are part of same observable.
             */
            struct IsOrthogonal {
                constexpr IsOrthogonal() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    // Not in same version of same observable, therefore not automatically orthogonal.
                    if ((lhs.observable != rhs.observable) || (lhs.flattenedSourceIndex != rhs.flattenedSourceIndex)) {
                        return false;
                    }
                    return (lhs.global_id != rhs.global_id);
                }
            };

            /**
             * Predicate: true if lhs == rhs, and both are part of same projective observable.
             */
            struct IsRedundant {
                constexpr IsRedundant() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    return lhs.projective && (lhs.global_id == rhs.global_id);
                }
            };
        };

        /** Augment base-network observable with extra info regarding inflated variants */
        struct ICObservable : public Observable {
        public:
            /** Information for an inflated variant of an observable */
            struct Variant {
            public:
                /** Global number of first operator in variant. */
                oper_name_t operator_offset;

                /** Single-number index of variant within observable. */
                oper_name_t flat_index;

                /** Index per source of variant within observable. */
                std::vector<oper_name_t> indices;

                /** Map, key: source ID, value: source variant. */
                std::map<oper_name_t, oper_name_t> source_variants;

                /** Bitmap, flagging which sources are connected to variant. */
                SourceListBitset connected_sources;

                /** True, if no overlapping sources, and not the same variant as other. */
                [[nodiscard]] bool independent(const Variant& other) const noexcept;

            friend class ::Moment::Inflation::InflationContext::ICObservable;
            private:
                Variant(oper_name_t operator_offset,
                        oper_name_t index,
                        std::vector<oper_name_t>&& vecIndex,
                        std::map<oper_name_t, oper_name_t>&& srcVariants,
                        SourceListBitset connected_sources);
            };

        private:
            const InflationContext& context;
        public:
            /** Global number of first operator in observable */
            const oper_name_t operator_offset;

            /** Global number of first variant in observable */
            const oper_name_t variant_offset;

            /** Number of variants of this observable */
            const oper_name_t variant_count;

            /** Variants of this observable */
            const std::vector<Variant> variants;

        public:
            ICObservable(const InflationContext& context, const Observable& baseObs,
                         size_t inflation_level, oper_name_t operator_offset, oper_name_t variant_offset);

            /** Get variant by non-flat index */
            [[nodiscard]] const Variant& variant(std::span<const oper_name_t> indices) const;

        private:
            static std::vector<Variant> make_variants(const CausalNetwork& network,
                                                      const Observable &baseObs,
                                                      size_t inflation_level, oper_name_t offset);
        };

    private:
        CausalNetwork base_network;
        size_t inflation;

        std::vector<ICOperatorInfo> operator_info;
        std::vector<ICObservable> inflated_observables;

        size_t total_inflated_observables = 0;

        size_t total_inflated_sources = 0;

        std::vector<OVIndex> global_variant_indices;

        /** Bitset, size equal to number of operators in context. True if other operator is not independent */
        std::vector<DynamicBitset<uint64_t>> dependent_operators;

    public:
        /**
         * Create a causal network context, for inflating.
         * @param network Causal network
         */
        InflationContext(CausalNetwork network, size_t inflation_level);

        /**
         * Vector of observables associated with context.
         */
        [[nodiscard]] const auto& Observables() const noexcept { return this->inflated_observables; }

        /**
         * Vector of sources associated with context.
         */
        [[nodiscard]] const auto& Sources() const noexcept { return this->base_network.Sources(); }

        /**
         * Level of inflation
         */
        [[nodiscard]] size_t Inflation() const noexcept { return this->inflation; }


        /**
         * Get total number of source variants
         */
        [[nodiscard]] size_t source_variant_count() const noexcept {
            return this->total_inflated_sources;
        }

        /**
        * Counts total number of variants of all observables
        */
        [[nodiscard]] size_t observable_variant_count() const noexcept {
            return this->total_inflated_observables;
        }

        /** False: as InflationContext never generates non-Hermitian operator strings. */
        [[nodiscard]] bool can_be_nonhermitian() const noexcept override { return false; }

         /**
          * Commute operators, check for idempotency, and check for orthogonal projectors.
          */
        bool additional_simplification(sequence_storage_t &op_sequence, bool &negate) const override;

        /**
         * Replace string with symmetric equivalent
         */
        OperatorSequence simplify_as_moment(OperatorSequence &&seq) const override;

         /**
          * Split operator sequence into smallest independent factors.
          */
        [[nodiscard]] std::vector<OperatorSequence> factorize(const OperatorSequence& seq) const;

        /**
         * Calculate equivalent variant of operator string with lowest possible source indices (e.g. 'A2' -> 'A0' etc.).
         */
        [[nodiscard]] OperatorSequence canonical_moment(const OperatorSequence& input) const;


        /**
         * Calculate equivalent variant of observables with lowest possible source indices (e.g. 'A2' -> 'A0' etc.).
         */
        [[nodiscard]] std::vector<OVIndex>
        canonical_variants(const std::vector<OVIndex>& input) const;

        /**
         * Unwrap outcome number to various outcomes of source measurements
         */
        [[nodiscard]] std::vector<OVOIndex>
        unflatten_outcome_index(std::span<const OVIndex> input, oper_name_t outcome_number) const;

        /**
         * Unwrap outcome number to various outcomes of source measurements
         */
        [[nodiscard]] size_t flatten_outcome_index(std::span<const OVOIndex> input) const;

        using Context::format_sequence;

        /**
         * Generates a formatted string representation of an operator sequence
         */
        [[nodiscard]] std::string format_sequence(const OperatorSequence& seq) const override;

        [[nodiscard]] std::string format_sequence(const std::vector<OVOIndex>& indices) const;


        /**
          * Get operator associated with following triplet:
          * @param observable
          * @param variant
          * @param outcome
          * @return The raw operator ID
          */
        [[nodiscard]] oper_name_t operator_number(oper_name_t observable, oper_name_t variant,
                                                  oper_name_t outcome) const noexcept;

        /**
         * Get a global observable variant index from the following pair:
         * @param observable
         * @param variant
         * @return The global variant index
         */
        [[nodiscard]] oper_name_t obs_variant_to_index(oper_name_t observable, oper_name_t variant) const;

        /**
         * Get a global observable variant index from the following pair:
         * @param observable
         * @param variant
         * @return The global variant index
         */
        [[nodiscard]] oper_name_t obs_variant_to_index(const OVIndex& index) const {
            return this->obs_variant_to_index(index.observable, index.variant);
        }

        /**
         * Get a global observable variant index from the following pair:
         * @return Pair, the observable and the variant thereof
         */
        [[nodiscard]] OVIndex index_to_obs_variant(oper_name_t global_variant_index) const;

        /**
         * Returns number of outcomes for each observable referred to by indices
         */
        [[nodiscard]] std::vector<size_t> outcomes_per_observable(std::span<const OVIndex> indices) const noexcept;

         /**
          * Output information about inflation context
          */
         [[nodiscard]] std::string to_string() const override;


        std::optional<OperatorSequence> get_if_canonical(const sequence_storage_t &sequence) const override;

        friend class InflationOperatorSequenceGenerator;

    protected:
        std::unique_ptr<OperatorSequenceGenerator> new_osg(size_t word_length) const override;
    };
}