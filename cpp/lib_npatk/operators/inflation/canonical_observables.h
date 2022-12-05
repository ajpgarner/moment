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
            std::vector<OVIndex> indices;
            size_t hash;

            CanonicalObservable(std::vector<OVIndex> index, size_t the_hash)
                : indices(std::move(index)), hash{the_hash} { }
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

        void generate_up_to_level(size_t new_level);

        [[nodiscard]] const CanonicalObservable& canonical(size_t hash) const;

        [[nodiscard]] const CanonicalObservable& canonical(std::span<const OVIndex> indices) const;

        [[nodiscard]] size_t distinct_observables(size_t level) const noexcept {
            assert(level < this->distinct_observables_per_level.size());
            return this->distinct_observables_per_level[level];
        }
    };
}