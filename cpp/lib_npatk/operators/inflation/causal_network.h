/**
 * causal_network.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Classical causal network.
 *
 */
#pragma once

#include "integer_types.h"

#include "observable.h"
#include "source.h"

#include <iosfwd>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace NPATK {

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