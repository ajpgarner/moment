/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

#include "utilities/alphabetic_namer.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace NPATK {

    std::vector<InflationContext::ICObservable::Variant>
    InflationContext::ICObservable::make_variants(const CausalNetwork& network, const Observable &baseObs,
                                                  const size_t inflation_level, const oper_name_t base_offset) {
        std::vector<InflationContext::ICObservable::Variant> output;
        const auto variant_count = static_cast<oper_name_t>(baseObs.count_copies(inflation_level));
        oper_name_t global_id = base_offset;

        for (oper_name_t variant_index = 0; variant_index < variant_count; ++variant_index) {
            auto vector_indices = baseObs.unflatten_index(inflation_level, variant_index);
            std::map<oper_name_t, oper_name_t> map_to_sources;
            DynamicBitset<uint64_t> sourceMap{inflation_level * network.Sources().size()};

            oper_name_t i = 0;
            for (auto source_id : baseObs.sources) {
                sourceMap.set((source_id * inflation_level) + vector_indices[i]);
                map_to_sources.emplace_hint(map_to_sources.end(), std::make_pair(source_id, vector_indices[i]));
                ++i;
            }

            output.emplace_back(InflationContext::ICObservable::Variant{global_id, variant_index, std::move(vector_indices),
                                                                        std::move(map_to_sources), sourceMap});

            global_id += static_cast<oper_name_t>(baseObs.outcomes-1);

        }
        return output;
    }

    InflationContext::ICObservable::Variant::Variant(const oper_name_t op_offset,
                                                     const oper_name_t index,
                                                     std::vector<oper_name_t> &&vecIndex,
                                                     std::map<oper_name_t, oper_name_t> &&srcVariants,
                                                     const DynamicBitset<uint64_t>& sourceBMP)
            : operator_offset{op_offset},  flat_index{index}, indices{std::move(vecIndex)},
              source_variants{std::move(srcVariants)}, connected_sources{sourceBMP} { }

    bool InflationContext::ICObservable::Variant::independent(const InflationContext::ICObservable::Variant &other)
        const noexcept {
        auto overlap = this->connected_sources & other.connected_sources;
        return overlap.empty();
    }


    InflationContext::ICObservable::ICObservable(const InflationContext& context,
                                                 const Observable &baseObs,
                                                 const size_t inflation_level,
                                                 const oper_name_t op_offset,
                                                 const oper_name_t var_offset)
         : Observable{baseObs}, context{context}, variant_count{static_cast<oper_name_t>(baseObs.count_copies(inflation_level))},
           operator_offset{op_offset}, variant_offset{var_offset},
           variants{make_variants(context.base_network, baseObs, inflation_level, op_offset)} {

    }

    const InflationContext::ICObservable::Variant &
    InflationContext::ICObservable::variant(std::span<const oper_name_t> indices) const {
        assert(indices.size() == this->source_count);
        size_t index = 0;
        size_t base = 1;
        for (size_t n = indices.size(); n > 0; --n) {
            index += indices[n-1] * base;
            base *= this->context.inflation;
        }
        assert(index < this->variants.size());
        return this->variants[index];
    }


    InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

        // Create operator and observable info
        this->inflated_observables.reserve(this->base_network.Observables().size());
        this->operator_info.reserve(this->size());
        this->total_inflated_observables = 0;
        this->global_variant_indices.clear();
        size_t obs_index = 0;
        oper_name_t global_id = 0;
        for (const auto& observable : this->base_network.Observables()) {
            this->inflated_observables.emplace_back(*this, observable, this->inflation,
                                                    global_id, this->total_inflated_observables);
            const auto num_copies = this->inflated_observables.back().variant_count;
            this->total_inflated_observables += num_copies;
            for (oper_name_t copy_index = 0; copy_index < num_copies; ++copy_index) {
                this->global_variant_indices.emplace_back(observable.id, copy_index);
                for (oper_name_t outcome = 0; outcome < (observable.outcomes-1); ++outcome) {
                    this->operator_info.emplace_back(global_id, observable.id, copy_index, outcome);
                    ++global_id;
                }
            }
            ++obs_index;
        }
        assert(this->operator_info.size() == this->size());
        assert(this->inflated_observables.size() == this->base_network.Observables().size());

        // Create independence maps
        this->dependent_operators.reserve(this->size());
        for (const auto& opInfo : this->operator_info) {
            const auto& obsInfo = this->inflated_observables[opInfo.observable];
            const auto& variant = obsInfo.variants[opInfo.flattenedSourceIndex];

            this->dependent_operators.emplace_back(this->size());
            auto& bitmap = this->dependent_operators.back();

            // Cycle through all observables, and test for independence...
            for (const auto& otherObs : this->inflated_observables) {
                const size_t operator_block_size = (otherObs.outcomes)-1;
                for (const auto& otherVariant : otherObs.variants) {
                    const bool independent = variant.independent(otherVariant);
                    if (!independent) {
                        const auto blitMax = otherVariant.operator_offset + operator_block_size;
                        for (size_t blitter = otherVariant.operator_offset; blitter < blitMax; ++blitter) {
                            bitmap.set(blitter);
                        }
                    }
                }
            }
        }
    }

    std::vector<OperatorSequence> InflationContext::factorize(const OperatorSequence& seq) const {
        std::vector<OperatorSequence> output;

        // If string of length 0 or 1, no further factorization is possible, just echo input sequence
        if (seq.size() <= 1) {
            output.push_back(seq);
            return output;
        }

        // Otherwise, try to factorize properly
        DynamicBitset<uint64_t> checklist{seq.size(), true};

        const auto total_source_count = this->Sources().size() * this->inflation;

        while (!checklist.empty()) {
            // Next string of unfactorized operators...
            std::vector<oper_name_t> opers{};
            DynamicBitset<uint64_t> included_sources{total_source_count};

            // Get next operator from input string that's not been put into any of the factors
            const size_t this_pos = checklist.first_index();
            checklist.unset(this_pos);
            assert(this_pos < seq.size());
            const oper_name_t this_oper_id = seq[this_pos];
            assert((this_oper_id >= 0) && (this_oper_id < this->operator_info.size()));
            const auto& this_oper_info = this->operator_info[this_oper_id];

            // Add operator to factor list
            opers.emplace_back(this_oper_id);

            // Flag sources that are included
            const auto& this_observable_variant =
                this->inflated_observables[this_oper_info.observable].variants[this_oper_info.flattenedSourceIndex];
            included_sources |= this_observable_variant.connected_sources;

            // Loop over remaining symbols in string, trying greedily to include them if any link is found
            bool done = false;
            while (!done) {
                done = true;
                for (size_t other_pos : checklist) {
                    assert(other_pos < seq.size());
                    const oper_name_t other_oper_id = seq[other_pos];
                    assert((other_oper_id >= 0) && (other_oper_id < this->operator_info.size()));
                    const auto& other_oper_info = this->operator_info[other_oper_id];
                    const auto& other_obs = this->inflated_observables[other_oper_info.observable];
                    const auto& other_obs_variant = other_obs.variants[other_oper_info.flattenedSourceIndex];

                    // Test for overlap
                    const bool overlap = !(included_sources & other_obs_variant.connected_sources).empty();
                    if (overlap) {
                        // Add to string
                        opers.emplace_back(other_oper_id);

                        // Included sources are now relevent
                        included_sources |= other_obs_variant.connected_sources;

                        // Operator is joined in list, so flag as handled.
                        checklist.unset(other_pos);

                        // Exit the for loop, but continue the while loop...
                        done = false;
                        break;
                    }
                }
                // For loop terminated without overlap, hence done == true and while loop terminates.
            }

            // Create operator sequence with what we have found
            output.emplace_back(std::move(opers), *this);
        }

        return output;
    }

    bool InflationContext::additional_simplification(std::vector<oper_name_t> &op_sequence, bool &negate) const {
        // Commutation between parties...
        std::vector<ICOperatorInfo> io_seq;
        io_seq.reserve(op_sequence.size());
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operator_count)) {
                throw std::range_error{"Operator ID higher than number of known operators."};
            }
            const auto& info = this->operator_info[op];
            io_seq.emplace_back(info);
        }

        // Completely commuting set, so sort (no need for stability)
        std::sort(io_seq.begin(), io_seq.end(), ICOperatorInfo::OrderByID{});

        // Check for nullity
        const ICOperatorInfo::IsOrthogonal isOrth;
        for (size_t index = 1, iMax = io_seq.size(); index < iMax; ++index) {
            if (isOrth(io_seq[index-1], io_seq[index])) {
                op_sequence.clear();
                return true;
            }
        }

        // Remove excess idempotent elements.
        auto trim_idem = std::unique(io_seq.begin(), io_seq.end(),
                                     ICOperatorInfo::IsRedundant{});
        io_seq.erase(trim_idem, io_seq.end());

        // Copy sequence to output
        op_sequence.clear();
        op_sequence.reserve(io_seq.size());
        for (const auto& op : io_seq) {
            op_sequence.emplace_back(op.global_id);
        }
        return false;
    }

    OperatorSequence InflationContext::simplify_as_moment(OperatorSequence &&seq) const {
        return this->canonical_moment(seq);
    }


    OperatorSequence InflationContext::canonical_moment(const OperatorSequence& input) const {
        // If 0 or I; or no inflation, just pass through
        if (input.empty() || (this->inflation <= 1)) {
            return input;
        }

        std::vector<oper_name_t> next_available_source(this->base_network.Sources().size(), 0);
        std::map<oper_name_t, oper_name_t> permutation{};
        std::vector<oper_name_t> permuted_operators;

        for (const oper_name_t op : input) {
            assert((op>=0) && (op < this->operator_count));
            const auto& op_info = this->operator_info[op];
            const auto& obs_info = this->inflated_observables[op_info.observable];
            const auto& variant_info = obs_info.variants[op_info.flattenedSourceIndex];

            std::vector<oper_name_t> source_indices;
            source_indices.reserve(variant_info.indices.size());

            for (auto src : variant_info.connected_sources) {
                auto permIter = permutation.find(static_cast<oper_name_t>(src));
                if (permIter != permutation.end()) {
                    // permutation already known
                    const oper_name_t new_src = permIter->second;
                    const oper_name_t new_variant = new_src % static_cast<oper_name_t>(this->inflation);
                    source_indices.emplace_back(new_variant);
                } else {
                    // new permutation required
                    const oper_name_t source = static_cast<oper_name_t>(src) / static_cast<oper_name_t>(this->inflation);
                    const oper_name_t new_variant = next_available_source[source];
                    ++next_available_source[source];
                    const oper_name_t new_src = (source * static_cast<oper_name_t>(this->inflation)) + new_variant;
                    permutation.emplace(std::make_pair(src, new_src));
                    source_indices.emplace_back(new_variant);
                }
            }
            const auto& new_variant_info = obs_info.variant(source_indices);
            const oper_name_t new_oper_id = new_variant_info.operator_offset + op_info.outcome;
            permuted_operators.push_back(new_oper_id);
        }
        return OperatorSequence{std::move(permuted_operators), *this};
    }

    std::vector<OVIndex>
    InflationContext::canonical_variants(const std::vector<OVIndex>& input) const {
        // If 0 or I; or no inflation, then nothing.
        if (input.empty() || (this->inflation < 1)) {
            return {};
        }

        std::vector<oper_name_t> next_available_source(this->base_network.Sources().size(), 0);
        std::map<oper_name_t, oper_name_t> permutation{};
        std::vector<OVIndex> permuted_variants;

        for (const auto [obs_id, var_id] : input) {
            assert((obs_id>=0) && (obs_id < this->inflated_observables.size()));
            const auto& obs_info = this->inflated_observables[obs_id];
            assert((var_id>=0) && (var_id < obs_info.variant_count));
            const auto& variant_info = obs_info.variants[var_id];

            std::vector<oper_name_t> source_indices;
            source_indices.reserve(variant_info.indices.size());

            for (auto src : variant_info.connected_sources) {
                auto permIter = permutation.find(static_cast<oper_name_t>(src));
                if (permIter != permutation.end()) {
                    // permutation already known
                    const oper_name_t new_src = permIter->second;
                    const oper_name_t new_variant = new_src % static_cast<oper_name_t>(this->inflation);
                    source_indices.emplace_back(new_variant);
                } else {
                    // new permutation required
                    const oper_name_t source = static_cast<oper_name_t>(src) / static_cast<oper_name_t>(this->inflation);
                    const oper_name_t new_variant = next_available_source[source];
                    ++next_available_source[source];
                    const oper_name_t new_src = (source * static_cast<oper_name_t>(this->inflation)) + new_variant;
                    permutation.emplace(std::make_pair(src, new_src));
                    source_indices.emplace_back(new_variant);
                }
            }
            const auto& new_variant_info = obs_info.variant(source_indices);
            permuted_variants.emplace_back(obs_id, new_variant_info.flat_index);
        }

        // Canonical sequence of variants is always sorted...
        std::sort(permuted_variants.begin(), permuted_variants.end());

        return permuted_variants;
    }



    oper_name_t InflationContext::operator_number(oper_name_t observable, oper_name_t variant,
                                                  oper_name_t outcome) const noexcept {
       assert((observable >= 0) && (observable < this->inflated_observables.size()));
       assert((observable >= 0) && (observable < this->base_network.Observables().size()));
       const auto& observable_info = this->inflated_observables[observable];
       assert((variant >= 0) && (variant < observable_info.variant_count));
       const auto& base_observable = this->base_network.Observables();
       return static_cast<oper_name_t>(observable_info.operator_offset)
                + (variant * (static_cast<oper_name_t>(this->base_network.Observables()[observable].outcomes)-1))
                + outcome;
   }

    oper_name_t InflationContext::obs_variant_to_index(const oper_name_t observable, const oper_name_t variant) const {
        assert((observable >= 0) && (observable < this->inflated_observables.size()));
        const auto& observable_info = this->inflated_observables[observable];
        assert((variant >= 0) && (variant < observable_info.variant_count));
        return observable_info.variant_offset + variant;
    }

    OVIndex
    InflationContext::index_to_obs_variant(const oper_name_t global_variant_index) const {
        assert((global_variant_index >= 0) && (global_variant_index < this->global_variant_indices.size()));
        return this->global_variant_indices[global_variant_index];
    }

    std::string InflationContext::format_sequence(const OperatorSequence &seq) const {
        if (seq.zero()) {
            return "0";
        }
        if (seq.empty()) {
            return "1";
        }

        std::stringstream ss;
        if (seq.negated()) {
            ss << "-";
        }

        AlphabeticNamer obsNamer{true};

        const bool needs_comma = this->inflation > 9;
        const bool needs_braces = std::any_of(this->Observables().cbegin(), this->Observables().cend(),
                                              [](const auto& obs) { return obs.outcomes > 2; });
        bool one_operator = false;
        for (const auto& oper : seq) {
            if (one_operator) {
                ss << ";";
            } else {
                one_operator = true;
            }

            if (oper > this->operator_info.size()) {
                ss << "[UNK:" << oper << "]";
            } else {
                const auto &extraInfo = this->operator_info[oper];
                const auto &obsInfo = this->inflated_observables[extraInfo.observable];

                ss << obsNamer(extraInfo.observable);
                if (obsInfo.outcomes > 2) {
                    ss << extraInfo.outcome;
                }
                // Give indices, if inflated
                if (this->inflation > 1) {
                    const auto& infIndices = obsInfo.variants[extraInfo.flattenedSourceIndex].indices;
                    bool done_one = false;
                    if (needs_braces) {
                        ss << "[";
                    }
                    for (auto infIndex : infIndices){
                        if (needs_comma && done_one) {
                            ss << ",";
                        } else {
                            done_one = true;
                        }
                        ss << infIndex;
                    }
                    if (needs_braces) {
                        ss << "]";
                    }
                }
            }
        }
        return ss.str();
    }

    std::vector<size_t> InflationContext::outcomes_per_observable(std::span<const OVIndex> indices) const noexcept {
        std::vector<size_t> output{};
        output.reserve(indices.size());
        for (const auto& index : indices) {
            assert(index.observable < this->inflated_observables.size());
            const auto& obs = this->inflated_observables[index.observable];
            output.push_back(obs.outcomes);
        }

        return output;
    }

    std::string InflationContext::to_string() const {
        std::stringstream ss;
        ss << "Inflation setting with "
           << this->operator_count << ((1 != this->operator_count) ? " operators" : " operator")
           << " in total.\n\n";
        ss << this->base_network << "\n";
        ss << "Inflation level: " << this->inflation;

        return ss.str();
    }



}
