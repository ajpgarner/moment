/**
 * canonical_observables.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "canonical_observables.h"

#include "inflation_context.h"

#include "utilities/combinations.h"

namespace NPATK {

    CanonicalObservables::CanonicalObservables(const InflationContext &context)
        : context{context}, max_level{0} {

    }

    void CanonicalObservables::generate_up_to_level(const size_t new_level) {
        // Do nothing, if already up to level
        if (new_level <= this->max_level) {
            return;
        }

        for (size_t level = this->max_level; level < new_level; ++level) {
            CombinationIndexIterator comboIter{context.observable_variant_count(), level};
            const CombinationIndexIterator comboIterEnd{context.observable_variant_count(), level, true};

            std::vector<std::pair<oper_name_t, oper_name_t>> obs_var_indices;
            obs_var_indices.reserve(level);

            while (comboIter != comboIterEnd) {
                const auto& global_indices = *comboIter;
                obs_var_indices.clear();
                for (auto index : global_indices) {
                    obs_var_indices.push_back(context.index_to_obs_variant(static_cast<oper_name_t>(index)));
                }

                const auto canonical_index = context.canonical_variants(obs_var_indices);

                // TODO: use this information somehow

                ++comboIter;
            }
        }

        // Save level
        this->max_level = new_level;
    }
}