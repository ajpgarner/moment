/**
 * inflation_context.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_context.h"

#include "operators/operator_sequence.h"
#include "utilities/alphabetic_namer.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace NPATK {

    std::vector<InflationContext::ICObservable::Variant>
    InflationContext::ICObservable::make_variants(const CausalNetwork& network, const Observable &baseObs,
                                                  const size_t inflation_level) {
        std::vector<InflationContext::ICObservable::Variant> output;
        const auto variant_count = static_cast<oper_name_t>(baseObs.count_copies(inflation_level));

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

            output.emplace_back(InflationContext::ICObservable::Variant{variant_index, std::move(vector_indices),
                                                                        std::move(map_to_sources), sourceMap});

        }
        return output;
    }

    InflationContext::ICObservable::Variant::Variant(oper_name_t index, std::vector<oper_name_t> &&vecIndex,
                                                     std::map<oper_name_t, oper_name_t> &&srcVariants,
                                                     const DynamicBitset<uint64_t>& sourceBMP)
            : flat_index{index}, indices{std::move(vecIndex)},
              source_variants{std::move(srcVariants)}, connected_sources{sourceBMP} { }

    bool InflationContext::ICObservable::Variant::independent(const InflationContext::ICObservable::Variant &other)
        const noexcept {
        auto overlap = this->connected_sources & other.connected_sources;
        return overlap.empty();
    }


    InflationContext::ICObservable::ICObservable(const InflationContext& context,
                                                 const Observable &baseObs,
                                                 const size_t inflation_level,
                                                 const oper_name_t offset)
         : Observable{baseObs}, context{context}, variant_count{static_cast<oper_name_t>(baseObs.count_copies(inflation_level))},
           operator_offset{offset}, variants{make_variants(context.base_network, baseObs, inflation_level)} {

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
        oper_name_t global_id = 0;
        for (const auto& observable : this->base_network.Observables()) {
            this->inflated_observables.emplace_back(*this, observable, this->inflation, global_id);
            const auto num_copies = this->inflated_observables.back().variant_count;
            for (oper_name_t copy_index = 0; copy_index < num_copies; ++copy_index) {
                for (oper_name_t outcome = 0; outcome < (observable.outcomes-1); ++outcome) {
                    this->operator_info.emplace_back(global_id, observable.id, copy_index, outcome);
                    ++global_id;
                }
            }
        }
        assert(this->operator_info.size() == this->size());
        assert(this->inflated_observables.size() == this->base_network.Observables().size());
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

    std::string InflationContext::to_string() const {
        std::stringstream ss;
        ss << "Inflation setting with "
           << this->operators.size() << ((1 !=  this->operators.size()) ? " operators" : " operator")
           << " in total.\n\n";
        ss << this->base_network << "\n";
        ss << "Inflation level: " << this->inflation;

        return ss.str();
    }

    bool InflationContext::additional_simplification(std::vector<oper_name_t> &op_sequence, bool &negate) const {
        // Commutation between parties...
        std::vector<ICOperatorInfo> io_seq;
        io_seq.reserve(op_sequence.size());
        for (const auto& op : op_sequence) {
            if ((op < 0) || (op >= this->operators.size())) {
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

}
