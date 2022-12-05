/**
 * canonical_observables.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include "observable_variant_index.h"

#include <cassert>

#include <map>
#include <stdexcept>
#include <span>
#include <vector>

namespace NPATK {

    class InflationContext;

    namespace errors {
        class bad_ov_string : public std::range_error {
        public:
            explicit bad_ov_string(const std::string& what) : std::range_error{what} { }
        };
    }

    class CanonicalObservables {
    public:
        struct CanonicalObservable {
            /** Index, in list of canonical observables */
            size_t index;

            /** Index string, in terms of observable/variant */
            std::vector<OVIndex> indices;

            /** Hash of OVIndex string */
            size_t hash;

            /** Total number of associated operators */
            size_t operators;

            /** Total number of associated outputs (i.e. operators + implicit operators) */
            size_t outcomes;

            CanonicalObservable(size_t index, std::vector<OVIndex> index_list, size_t the_hash,
                                size_t ops, size_t outcomes)
                : index{index}, indices(std::move(index_list)), hash{the_hash}, operators{ops}, outcomes{outcomes} { }
        };

    private:
        const InflationContext& context;
        size_t max_level = 0;

        std::vector<size_t> distinct_observables_per_level;

        std::vector<CanonicalObservable> canonical_observables;

        std::map<size_t, size_t> hash_aliases;

    public:
        explicit CanonicalObservables(const InflationContext& context);

        /**
         * Hash a string of OV indices
         */
        [[nodiscard]] size_t hash(std::span<const OVIndex> index) const;

        [[nodiscard]] size_t hash(std::span<const size_t> global_index) const;

        void generate_up_to_level(size_t new_level);

        [[nodiscard]] const CanonicalObservable& canonical(size_t hash) const;

        [[nodiscard]] const CanonicalObservable& canonical(std::span<const OVIndex> indices) const;

        [[nodiscard]] const CanonicalObservable& canonical(std::span<const size_t> indices) const;


        [[nodiscard]] size_t distinct_observables(size_t level) const noexcept {
            assert(level < this->distinct_observables_per_level.size());
            return this->distinct_observables_per_level[level];
        }

        [[nodiscard]] auto begin() const noexcept { return this->canonical_observables.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->canonical_observables.cend(); }
        [[nodiscard]] auto size() const noexcept { return this->canonical_observables.size(); }

    };
}