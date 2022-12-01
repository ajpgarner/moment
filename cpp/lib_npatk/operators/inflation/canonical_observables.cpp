/**
 * canonical_observables.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "canonical_observables.h"

#include "inflation_context.h"

#include "utilities/combinations.h"

#include <sstream>

namespace NPATK {

    CanonicalObservables::CanonicalObservables(const InflationContext &context)
        : context{context}, max_level{0} {
        // One at level 0 (ID)
        this->distinct_observables_per_level.emplace_back(1);

        std::vector<OVIndex> empty{};
        const auto empty_hash = this->context.ov_hash(empty);
        const auto empty_canon = this->context.canonical_variants(empty);
        const auto empty_canon_hash = this->context.ov_hash(empty_canon);
        this->hash_aliases.emplace(std::make_pair(empty_hash, empty_canon_hash));
    }

    void CanonicalObservables::generate_up_to_level(const size_t new_level) {
        // Do nothing, if already up to level
        if (new_level <= this->max_level) {
            return;
        }

        for (size_t level = this->max_level+1; level <= new_level; ++level) {
            CombinationIndexIterator comboIter{context.observable_variant_count(), level};
            const CombinationIndexIterator comboIterEnd{context.observable_variant_count(), level, true};

            std::vector<OVIndex> obs_var_indices;
            obs_var_indices.reserve(level);

            std::set<size_t> hashes_this_level{};

            while (comboIter != comboIterEnd) {
                const auto& global_indices = *comboIter;
                obs_var_indices.clear();
                for (auto index : global_indices) {
                    obs_var_indices.push_back(context.index_to_obs_variant(static_cast<oper_name_t>(index)));
                }

                size_t raw_hash = context.ov_hash(obs_var_indices);

                const auto canonical_indices = context.canonical_variants(obs_var_indices);
                size_t canonical_hash = context.ov_hash(canonical_indices);

                this->hash_aliases.emplace(std::make_pair(raw_hash, canonical_hash));
                hashes_this_level.emplace(canonical_hash);

                ++comboIter;
            }
            this->distinct_observables_per_level.emplace_back(hashes_this_level.size());
        }

        // Save level
        this->max_level = new_level;
    }

    size_t CanonicalObservables::canonical(const size_t hash) const {
        auto iter = this->hash_aliases.find(hash);
        if (iter == this->hash_aliases.cend()) {
            throw errors::bad_ov_string{"Could not find string in hash table."};
        }
        return iter->second;
    }

    size_t  CanonicalObservables::canonical(std::span<const OVIndex> indices) const {
        try {
            if (indices.size() > this->max_level) {
                throw errors::bad_ov_string{"String is too long."};
            }
            const auto raw_hash = context.ov_hash(indices);
            return this->canonical(raw_hash);
        } catch (const errors::bad_ov_string& e) {
            std::stringstream ss;
            ss << "Error with string \"";
            for (const auto& index : indices) {
                ss << index;
            }
            ss << "\": " << e.what();

            // Rethrow with string info
            throw errors::bad_ov_string{ss.str()};
        }
    }
}