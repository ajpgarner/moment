/**
 * causal_network.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Classical causal network.
 *
 */
#pragma once

#include "integer_types.h"

#include "observable.h"
#include "source.h"

#include <cassert>
#include <cstdlib>

#include <iosfwd>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <span>
#include <vector>

namespace Moment::Inflation {

    namespace errors {
        class bad_observable : public std::logic_error {
        public:
            size_t index;
            bad_observable(size_t i, const std::string& what) : std::logic_error{what}, index{i} { }
        };

        class bad_source : public std::logic_error {
        public:
            size_t index;
            bad_source(size_t i, const std::string& what) : std::logic_error{what}, index{i} { }
        };
    }

    class CausalNetwork {
    private:
        /** Classical measurements */
        std::vector<Observable> observables;

        /** Hidden variables connecting measurements */
        std::vector<Source> sources;

        /** Index of first implicit source */
        size_t implicit_source_index;

    public:
        /**
          * Create a causal network
          * @param observable_init_list Vector: number of outcomes per observable
          * @param source_init_list Vector: sets of observables linked to from each source
          */
        CausalNetwork(const std::vector<size_t>& observable_init_list,
                      std::vector<std::set<oper_name_t>>&& source_init_list);

        CausalNetwork(const CausalNetwork& rhs) = delete;

        CausalNetwork(CausalNetwork&& rhs) = default;

        /**
         * Vector of observables associated with context.
         */
        [[nodiscard]] const auto& Observables() const noexcept { return this->observables; }

        /**
         * Vector of sources associated with context.
         */
        [[nodiscard]] const auto& Sources() const noexcept { return this->sources; }

        /**
         * Calculate total number of sources for this network at a particular inflation level
         */
        [[nodiscard]] size_t total_source_count(const size_t inflation_level) const noexcept {
            const size_t explicit_sources = this->implicit_source_index;
            const size_t implicit_sources = this->sources.size() - explicit_sources;
            return (explicit_sources * inflation_level) + implicit_sources;
        }

        /**
         * Get the number of explicitly specified sources.
         */
        [[nodiscard]] size_t explicit_source_count() const noexcept {
            return this->implicit_source_index;
        }

        /**
         * Get the number of implicitly added sources.
         */
        [[nodiscard]] size_t implicit_source_count() const noexcept {
            return this->sources.size() - this->implicit_source_index;
        }


        /**
         * Convert global source number to source and variant number.
         */
         [[nodiscard]] std::pair<size_t, size_t>
         global_source_to_source_variant(const size_t inflation_level,  size_t global_id) const noexcept {
             if (global_id > (this->implicit_source_index * inflation_level)) {
                 return {global_id + this->implicit_source_index * (1 - inflation_level), 0};
             }

             // Otherwise division
             return { global_id / inflation_level, global_id % inflation_level};
         }

         /**
         * Convert source and variant number to global number
         */
         [[nodiscard]] size_t
         source_variant_to_global_source(const size_t inflation_level,
                                         const size_t source_id, const size_t variant_id) const noexcept {
             if (source_id >= this->implicit_source_index) {
                 assert(variant_id == 0);
                 return (this->implicit_source_index * inflation_level) + (source_id - this->implicit_source_index);
             }
             return (source_id * inflation_level) + variant_id;
         }

         /**
          * Apply permutation on index array
          * @param inflation The inflation level.
          * @param source_ids The name of each symbol referred to by the index.
          * @param source_permutation The permutation map
          * @param old_source_indices The index to permute
          * @return Permuted index array.
          */
        SourceIndex permute_variant(const size_t inflation, std::span<const oper_name_t> source_ids,
                                    const std::map<oper_name_t, oper_name_t>& source_permutation,
                                    std::span<const oper_name_t> old_source_indices) const;

        /**
         * Calculate total number of operators required for expressing this network at a particular inflation level
         */
        [[nodiscard]] size_t total_operator_count(size_t inflation_level) const noexcept;


        friend std::ostream& operator<<(std::ostream& os, const CausalNetwork& network);

    private:
        /**
          * Verify that observables and sources are well defined (throw otherwise)
          * @param max_observables Number of observables
          * @param source_init_list List of observables connected to each source.
          * @throws bad_source If observable out of range
          * @return List of sources connected to each observable
          */
        [[nodiscard]] static std::vector<std::set<oper_name_t>>
        reverse_observable_to_source(size_t max_observables,
                                     const std::vector<std::set<oper_name_t>>& source_init_list);
    };
}