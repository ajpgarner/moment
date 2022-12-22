/**
 * causal_network.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "causal_network.h"
#include "utilities/alphabetic_namer.h"

#include <cassert>

#include <sstream>

namespace Moment {
    CausalNetwork::CausalNetwork(const std::vector<size_t> &observable_init_list,
                                 std::vector<std::set<oper_name_t>> &&source_init_list)
             : implicit_source_index{source_init_list.size()} {
        // Check sources, and get reverse list
        auto observable_source_sets = CausalNetwork::reverse_observable_to_source(observable_init_list.size(),
                                                                                  source_init_list);

        // List of singleton observables, that will receive implicit sources (necessary for correct factorization!)
        std::vector<oper_name_t> singleton_observables;
        size_t next_implicit_source = this->implicit_source_index;

        // Make observables
        this->observables.reserve(observable_init_list.size());
        for (size_t o = 0, oMax = observable_init_list.size(); o < oMax; ++o) {
            // Singleton, if no sources map to observable
            const bool singleton = observable_source_sets[o].empty();
            if (singleton) {
                singleton_observables.emplace_back(static_cast<oper_name_t>(o));
                observable_source_sets[o].emplace(next_implicit_source);
                ++next_implicit_source;
            }

            this->observables.emplace_back(static_cast<oper_name_t>(o), observable_init_list[o],
                                           std::move(observable_source_sets[o]), singleton);
        }
        observable_source_sets.clear();

        // Make explicit sources
        this->sources.reserve(source_init_list.size());
        for (size_t s = 0, sMax = source_init_list.size(); s < sMax; ++s) {
            this->sources.emplace_back(static_cast<oper_name_t>(s), std::move(source_init_list[s]), false);
        }
        source_init_list.clear();

        // Now, add implicit sources
        for (const oper_name_t obs : singleton_observables) {
            this->sources.emplace_back(this->sources.size(), std::set<oper_name_t>{obs}, true);
        }
        assert(this->sources.size() == next_implicit_source);
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

    size_t CausalNetwork::total_source_count(size_t inflation_level) const noexcept {
        const size_t explicit_sources = this->implicit_source_index;
        const size_t implicit_sources = this->sources.size() - explicit_sources;
        return (explicit_sources * inflation_level) + implicit_sources;
    }

    size_t CausalNetwork::total_operator_count(size_t inflation_level) const noexcept {
        size_t output = 0;
        for (const auto& ob : this->observables) {
            output += ob.count_operators(inflation_level);
        }
        return output;
    }

    std::ostream& operator<<(std::ostream& os, const CausalNetwork& network) {
        AlphabeticNamer observable_namer{true};
        const size_t oMax = network.observables.size();
        const size_t sMax = network.sources.size();

        os << "Causal network with " << oMax << ((1 != oMax) ? " observables" : " observable")
           << " and " << sMax << ((1 != sMax) ? " sources" : " source") << ".\n";

        // List observables
        for (size_t oIndex = 0; oIndex < oMax; ++oIndex) {
            const auto& observable = network.observables[oIndex];
            os << "Observable " << observable_namer(observable.id) << " ["
               << observable.outcomes << "]";
            if (!observable.sources.empty()) {
                os << " <- ";
                bool done_one = false;
                for (const auto s : observable.sources) {
                    if (done_one) {
                        os << ", ";
                    } else {
                        done_one = true;
                    }
                    os << s;
                }
            }
            os << "\n";
        }

        // List sources
        for (size_t sIndex = 0; sIndex < sMax; ++sIndex) {
            const auto& source = network.sources[sIndex];
            os << "Source " << source.id;
            if (!source.observables.empty()) {
                os << " -> ";
                bool done_one = false;
                for (const auto o : source.observables) {
                    if (done_one) {
                        os << ", ";
                    } else {
                        done_one = true;
                    }
                    os << observable_namer(o);
                }
            }
            os << "\n";
        }

        return os;
    }
}
