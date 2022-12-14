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

    struct CanonicalObservable {
        /** Index, in list of canonical observables */
        size_t index;

        /** Index string, in terms of observable/variant */
        std::vector<OVIndex> indices;

        /** Index string, flattened */
        std::vector<size_t> flattened_indices;

        /** Hash of OVIndex string */
        size_t hash;

        /** Total number of associated operators */
        size_t operators;

        /** Total number of associated outputs (i.e. operators + implicit operators) */
        size_t outcomes;

        CanonicalObservable(size_t index, std::vector<OVIndex> index_list, std::vector<size_t> flat_index_list,
                            size_t the_hash, size_t ops, size_t outcomes)
                : index{index}, indices(std::move(index_list)), flattened_indices(std::move(flat_index_list)),
                  hash{the_hash}, operators{ops}, outcomes{outcomes} { }

        /** String length of the canonical observable */
        [[nodiscard]] constexpr auto size() const noexcept { return this->indices.size(); }

        /** Does the canonical observable have a string length of zero (i.e. represents normalization) */
        [[nodiscard]] constexpr auto empty() const noexcept { return this->indices.empty(); }
    };

    class CanonicalObservables {
    private:
        const InflationContext& context;
        size_t max_level = 0;

        std::vector<size_t> distinct_observables_per_level;

        std::vector<CanonicalObservable> canonical_observables;

        std::map<size_t, size_t> hash_aliases;

    public:
        explicit CanonicalObservables(const InflationContext& context);

        /** Hash a string of OV indices */
        [[nodiscard]] size_t hash(std::span<const OVIndex> index) const;

        /** Hash a string of OVO indices (outcome is ignored) */
        [[nodiscard]] size_t hash(std::span<const OVOIndex> index) const;

        [[nodiscard]] size_t hash(std::span<const size_t> global_index) const;

        void generate_up_to_level(size_t new_level);

        /** Look up canonical observable associated with particular hash */
        [[nodiscard]] const CanonicalObservable& canonical(size_t hash) const;

        /** Look up canonical observable associated with particular OV index string */
        [[nodiscard]] const CanonicalObservable& canonical(std::span<const OVIndex> indices) const;

        /** Look up canonical observable associated with particular OVO index string (outcome is ignored) */
        [[nodiscard]] const CanonicalObservable& canonical(std::span<const OVOIndex> indices) const;

        /** Look up canonical observable associated with particular raw variant number string */
        [[nodiscard]] const CanonicalObservable& canonical(std::span<const size_t> indices) const;

        [[nodiscard]] size_t distinct_observables(size_t level) const noexcept {
            assert(level < this->distinct_observables_per_level.size());
            return this->distinct_observables_per_level[level];
        }

        [[nodiscard]] auto begin() const noexcept { return this->canonical_observables.cbegin(); }

        [[nodiscard]] auto end() const noexcept { return this->canonical_observables.cend(); }

        [[nodiscard]] constexpr size_t size() const noexcept { return this->canonical_observables.size(); }

        [[nodiscard]] constexpr const CanonicalObservable& operator[](const size_t index) const {
            assert(index < this->size());
            return this->canonical_observables[index];
        }

    };
}