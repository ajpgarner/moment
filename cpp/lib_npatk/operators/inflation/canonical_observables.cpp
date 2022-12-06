/**
 * canonical_observables.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "canonical_observables.h"

#include "inflation_context.h"

#include "utilities/combinations.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <sstream>

namespace NPATK {

    CanonicalObservables::CanonicalObservables(const InflationContext &context)
        : context{context}, max_level{0} {

        // One at level 0 (ID)
        this->canonical_observables.emplace_back(0, std::vector<OVIndex>{}, 0, 1, 1);
        this->hash_aliases.emplace(std::make_pair(0ULL, 0ULL));
        this->distinct_observables_per_level.emplace_back(1);
    }

    void CanonicalObservables::generate_up_to_level(const size_t new_level) {
        // Do nothing, if already up to level
        if (new_level <= this->max_level) {
            return;
        }

        // Add new entries...
        for (size_t level = this->max_level+1; level <= new_level; ++level) {
            const size_t unique_observables_at_start = this->canonical_observables.size();

            CombinationIndexIterator comboIter{context.observable_variant_count(), level};
            const CombinationIndexIterator comboIterEnd{context.observable_variant_count(), level, true};

            std::vector<OVIndex> obs_var_indices;
            obs_var_indices.reserve(level);
            while (comboIter != comboIterEnd) {
                // Make raw string
                const auto& global_indices = *comboIter;
                obs_var_indices.clear();
                for (auto index : global_indices) {
                    obs_var_indices.push_back(context.index_to_obs_variant(static_cast<oper_name_t>(index)));
                }

                // Get raw hash
                const size_t raw_hash = this->hash(obs_var_indices);

                // Get canonical string & hash
                auto canonical_indices = context.canonical_variants(obs_var_indices);
                size_t canonical_hash = this->hash(canonical_indices);

                // Make sure canonical entry exists
                auto canonicalEntryIter = this->hash_aliases.find(canonical_hash);
                size_t the_index = 0;
                if (canonicalEntryIter == this->hash_aliases.cend()) {
                    size_t op_count = 1;
                    size_t out_count = 1;
                    for (const auto& cv : canonical_indices) {
                        const auto cvOutcomes = context.Observables()[cv.observable].outcomes;
                        op_count *= (cvOutcomes - 1);
                        out_count *= cvOutcomes;
                    }

                    // new hash
                    this->canonical_observables.emplace_back(this->canonical_observables.size(),
                                                             std::move(canonical_indices), canonical_hash,
                                                             op_count, out_count);


                    the_index = this->canonical_observables.size() - 1;
                    this->hash_aliases.emplace(std::make_pair(canonical_hash, the_index));
                } else {
                    the_index = canonicalEntryIter->second;
                }

                // Add hash alias
                this->hash_aliases.emplace(std::make_pair(raw_hash, the_index));

                ++comboIter;
            }
            this->distinct_observables_per_level.emplace_back(this->canonical_observables.size()
                                                              - unique_observables_at_start);
        }

        // Save level
        this->max_level = new_level;
    }

    size_t CanonicalObservables::hash(std::span<const OVIndex> indices) const {
        size_t multiplier = 1;
        size_t hash = 0;
        for (auto rIter = indices.rbegin(); rIter != indices.rend(); ++rIter) {
            const auto& index = *rIter;
            hash += (1+this->context.obs_variant_to_index(index)) * multiplier;
            multiplier *= (this->context.observable_variant_count());
        }
        return hash;
    }

    size_t CanonicalObservables::hash(std::span<const size_t> global_indices) const {
        size_t multiplier = 1;
        size_t hash = 0;
        for (auto rIter = global_indices.rbegin(); rIter != global_indices.rend(); ++rIter) {
            const auto& index = *rIter;
            hash += (1+index) * multiplier;
            multiplier *= (this->context.observable_variant_count());
        }
        return hash;
    }


    const CanonicalObservable& CanonicalObservables::canonical(const size_t hash) const {
       auto iter = this->hash_aliases.find(hash);
       if (iter == this->hash_aliases.cend()) {
           throw errors::bad_ov_string{"Could not find string in hash table."};
       }

       const auto index = iter->second;
       assert(index < this->canonical_observables.size());
       return this->canonical_observables[index];
    }

    const CanonicalObservable& CanonicalObservables::canonical(std::span<const OVIndex> indices) const {
        try {
            if (indices.size() > this->max_level) {
                throw errors::bad_ov_string{"String is too long."};
            }
            const auto raw_hash = this->hash(indices);
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

    const CanonicalObservable& CanonicalObservables::canonical(std::span<const size_t> indices) const {
        try {
            if (indices.size() > this->max_level) {
                throw errors::bad_ov_string{"String is too long."};
            }
            const auto raw_hash = this->hash(indices);
            return this->canonical(raw_hash);
        } catch (const errors::bad_ov_string& e) {
            std::stringstream ss;
            ss << "Error with indices \"";
            bool done_one = false;
            for (const auto& index : indices) {
                if (done_one) {
                    ss << ", ";
                }
                ss << index;
                done_one = true;
            }
            ss << "\": " << e.what();

            // Rethrow with string info
            throw errors::bad_ov_string{ss.str()};
        }
    }
}