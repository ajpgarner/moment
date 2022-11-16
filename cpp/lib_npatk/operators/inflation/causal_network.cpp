/**
 * causal_network.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "causal_network.h"

#include <sstream>

namespace NPATK {
    CausalNetwork::CausalNetwork(const std::vector<size_t> &observable_init_list,
                                       std::vector<std::set<oper_name_t>> &&source_init_list) {

        // Check observable outcome counts
        for (size_t o = 0, oMax = observable_init_list.size(); o < oMax; ++o) {
            if (observable_init_list[o] <= 0) {
                std::stringstream errSS;
                errSS << "Observable " << o << " must have at least one outcome.";
                throw errors::bad_observable(o, errSS.str());
            }
        }

        // Check sources, and get reverse list
        auto observable_source_sets = CausalNetwork::reverse_observable_to_source(observable_init_list.size(),
                                                                                  source_init_list);

        // Make observables
        this->observables.reserve(observable_init_list.size());
        for (size_t o = 0, oMax = observable_init_list.size(); o < oMax; ++o) {
            this->observables.emplace_back(static_cast<oper_name_t>(o), observable_init_list[o],
                                           std::move(observable_source_sets[o]));
        }
        observable_source_sets.clear();

        // Make sources
        this->sources.reserve(source_init_list.size());
        for (size_t s = 0, sMax = source_init_list.size(); s < sMax; ++s) {
            this->sources.emplace_back(static_cast<oper_name_t>(s), std::move(source_init_list[s]));
        }
        source_init_list.clear();
    }

    std::vector<std::set<oper_name_t>>
    CausalNetwork::reverse_observable_to_source(const size_t num_observables,
                                                const std::vector<std::set<oper_name_t>>& sources) {
        // Prepare output
        std::vector<std::set<oper_name_t>> output(num_observables, std::set<oper_name_t>{});

        for (size_t s = 0, sMax = sources.size(); s < sMax; ++s) {
            const auto &sourceSet = sources[s];
            for (auto oId: sourceSet) {
                // Check sources are in bounds
                if ((oId < 0) || (oId >= num_observables)) {
                    std::stringstream errSS;
                    errSS << "Source " << s << " maps to out of bound observable " << oId;
                    throw errors::bad_source(s, errSS.str());
                }
                // Add to set
                output[oId].emplace(s);
            }
        }

        return output;

    }

    size_t CausalNetwork::total_operator_count(size_t inflation_level) const noexcept {
        size_t output = 0;
        for (const auto& ob : this->observables) {
            output += ob.count_operators(inflation_level);
        }
        return output;
    }
}

