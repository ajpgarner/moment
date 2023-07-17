/**
 * canonical_observables.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "observable_variant_index.h"

#include <cassert>

#include <iosfwd>
#include <map>
#include <stdexcept>
#include <span>
#include <vector>

namespace Moment::Inflation {

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

        /** True, if all constituent parts are projective */
        bool projective;

        /** Hash of OVIndex string */
        size_t hash;

        /** Total number of associated operators */
        size_t operators;

        /** Total number of associated outputs (i.e. operators + implicit operators) */
        size_t outcomes;

        /** Number of outcomes for each associated measurement */
        std::vector<size_t> outcomes_per_observable;

        CanonicalObservable(size_t index, std::vector<OVIndex> index_list, std::vector<size_t> flat_index_list,
                            bool projective, size_t the_hash, size_t ops, size_t outcomes, std::vector<size_t> opo)
                : index{index}, indices(std::move(index_list)), flattened_indices(std::move(flat_index_list)),
                  projective{projective}, hash{the_hash}, operators{ops}, outcomes{outcomes},
                  outcomes_per_observable(std::move(opo)) { }

        /** String length of the canonical observable */
        [[nodiscard]] constexpr auto size() const noexcept { return this->indices.size(); }

        /** Does the canonical observable have a string length of zero (i.e. represents normalization) */
        [[nodiscard]] constexpr auto empty() const noexcept { return this->indices.empty(); }
    };

    /**
     * When inflating a causal network, many redundant observables are generated.
     * For instance, <A_00>, <A_01>, <A_10> and <A_11> should all give the same statistics (that of uninflated <A>).
     * Likewise <A_0 B_0> sharing a source should give the same statistics as <A_1 B_1> (also sharing a source).
     * This class keeps track of such sets of aliased observables, and labels each set by a single 'canonical'
     * observable, with the 'lowest' index.
     */
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

        /** Hash according to global index */
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

    private:
        void generate_level_projective(size_t new_level);

        void generate_level_nonprojective(size_t new_level);

        void try_add_entry(const size_t level, const std::vector<size_t>& global_indices);

        friend std::ostream& operator<<(std::ostream& os, const CanonicalObservables& co);

    };
}