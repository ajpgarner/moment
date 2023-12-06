/**
 * inflation_context.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "inflation_context.h"

#include "inflation_osg.h"

#include "utilities/alphabetic_namer.h"
#include "utilities/dynamic_bitset.h"
#include "utilities/small_vector.h"

#include <cassert>

#include <algorithm>
#include <bit>
#include <sstream>

namespace Moment::Inflation {

    namespace {
        using OpStringBitset = DynamicBitset<uint64_t, size_t, SmallVector<uint64_t, 1>>;


        /** 'Scratch' permutation paper, per thread. */
        inline std::vector<oper_name_t>& get_permutation_scratch(size_t size) {
            thread_local std::vector<oper_name_t> permutation_scratch;
            if (permutation_scratch.size() != size) {
                permutation_scratch.resize(size);
            }
            return permutation_scratch;
        }
    }

    std::vector<InflationContext::ICObservable::Variant>
    InflationContext::ICObservable::make_variants(const CausalNetwork& network, const Observable &baseObs,
                                                  const size_t inflation_level, const oper_name_t base_offset) {
        std::vector<InflationContext::ICObservable::Variant> output;
        const auto variant_count = static_cast<oper_name_t>(baseObs.count_copies(inflation_level));
        oper_name_t global_id = base_offset;

        for (oper_name_t variant_index = 0; variant_index < variant_count; ++variant_index) {
            assert(!baseObs.singleton || variant_index == 0);
            auto vector_indices = baseObs.unflatten_index(inflation_level, variant_index);
            std::map<oper_name_t, oper_name_t> map_to_sources;

            SourceListBitset sourceMap{network.total_source_count(inflation_level)};
            oper_name_t i = 0;
            for (auto source_id : baseObs.sources) {
                sourceMap.set(network.source_variant_to_global_source(inflation_level, source_id, vector_indices[i]));
                map_to_sources.emplace_hint(map_to_sources.end(), std::make_pair(source_id, vector_indices[i]));
                ++i;
            }

            output.emplace_back(InflationContext::ICObservable::Variant{global_id, variant_index, std::move(vector_indices),
                                                                        std::move(map_to_sources), std::move(sourceMap)});

            global_id += static_cast<oper_name_t>(baseObs.operators());
        }
        return output;
    }

    InflationContext::ICObservable::Variant::Variant(const oper_name_t op_offset,
                                                     const oper_name_t index,
                                                     SourceIndex&& vecIndex,
                                                     std::map<oper_name_t, oper_name_t> &&srcVariants,
                                                     SourceListBitset sourceBMP)
            : operator_offset{op_offset},  flat_index{index}, indices{std::move(vecIndex)},
              source_variants{std::move(srcVariants)}, connected_sources{std::move(sourceBMP)} { }

    bool InflationContext::ICObservable::Variant::independent(const InflationContext::ICObservable::Variant &other)
        const noexcept {
        // If singleton, independent with everything except for itself
        if (this->source_variants.empty()) {
            return this->operator_offset != other.operator_offset;
        }

        // Otherwise, independent if no common sources
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
        const auto index = this->flatten_index(this->context.inflation, indices);
        assert(index < this->variants.size());
        return this->variants[index];
    }




    InflationContext::InflationContext(CausalNetwork network, size_t inflation_level)
        : Context{network.total_operator_count(inflation_level)},
          base_network{std::move(network)},
          inflation{inflation_level} {

        // Query for source count
        this->total_inflated_sources = this->base_network.total_source_count(inflation);

        // Create operator and observable info
        this->inflated_observables.reserve(this->base_network.Observables().size());
        this->operator_info.reserve(this->size());
        this->total_inflated_observables = 0;
        this->global_variant_indices.clear();
        size_t obs_index = 0;
        oper_name_t global_id = 0;
        for (const auto& observable : this->base_network.Observables()) {
            this->inflated_observables.emplace_back(*this, observable,
                                         this->inflation,global_id,
                                                    static_cast<oper_name_t>(this->total_inflated_observables));
            const auto num_variants = this->inflated_observables.back().variant_count;
            this->total_inflated_observables += num_variants;
            for (oper_name_t variant_index = 0; variant_index < num_variants; ++variant_index) {
                this->global_variant_indices.emplace_back(observable.id, variant_index);
                for (oper_name_t outcome = 0; outcome < observable.operators(); ++outcome) {
                    this->operator_info.emplace_back(global_id, observable.id, variant_index,
                                                     outcome, observable.projective());
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
            const auto& variant = obsInfo.variants[opInfo.variant];

            this->dependent_operators.emplace_back(this->size());
            auto& bitmap = this->dependent_operators.back();

            // Cycle through all observables, and test for independence...
            for (const auto& otherObs : this->inflated_observables) {
                const size_t operator_block_size = otherObs.operators();
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
        OpStringBitset checklist{seq.size(), true};

        const auto total_source_count = this->total_inflated_sources; // this->Sources().size() * this->inflation;

        while (!checklist.empty()) {
            // Next string of unfactorized operators...
            sequence_storage_t opers{};
            SourceListBitset included_sources{total_source_count};

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
                this->inflated_observables[this_oper_info.observable].variants[this_oper_info.variant];
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
                    const auto& other_obs_variant = other_obs.variants[other_oper_info.variant];

                    // Test for overlap
                    const bool overlap = !(included_sources & other_obs_variant.connected_sources).empty();
                    if (overlap) {
                        // Add to string
                        opers.emplace_back(other_oper_id);

                        // Included sources are now relevant
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

    bool InflationContext::additional_simplification(sequence_storage_t &op_sequence, SequenceSignType& sign_type) const {

        // Completely commuting set, so sort (no need for stability)
        std::sort(op_sequence.begin(), op_sequence.end());

        // Check for nullity
        const ICOperatorInfo::IsOrthogonal isOrth;
        for (size_t index = 1, iMax = op_sequence.size(); index < iMax; ++index) {
            const auto& lhs = this->operator_info[op_sequence[index-1]];
            const auto& rhs = this->operator_info[op_sequence[index]];
            if (isOrth(lhs, rhs)) {
                op_sequence.clear();
                return true;
            }
        }

        // Remove excess idempotent elements.
        auto redundancy_checker = [&](oper_name_t lhs, oper_name_t rhs) {
            return (lhs == rhs) && this->operator_info[lhs].projective;
        };
        auto trim_idem = std::unique(op_sequence.begin(), op_sequence.end(), redundancy_checker);
        op_sequence.erase(trim_idem, op_sequence.end());

        return false;
    }


    std::optional<OperatorSequence> InflationContext::get_if_canonical(const sequence_storage_t &sequence) const {
        // Sequences commute, so canonical variations are sorted.
        if (!std::is_sorted(sequence.cbegin(), sequence.cend())) {
            return std::nullopt;
        }

        // If any adjacent elements from same measurement, then sequence is not unique [either projects out, or nullifies]
        const ICOperatorInfo::IsOrthogonal isOrth;
        for (size_t index = 1, iMax = sequence.size(); index < iMax; ++index) {
            const auto& lhs = this->operator_info[sequence[index-1]];
            const auto& rhs = this->operator_info[sequence[index]];
            // A0A1 = 0 -> not canonical
            if (isOrth(lhs, rhs)) {
                return std::nullopt;
            }
            // A^2 = A -> not canonical
            if (lhs.projective && (sequence[index-1] == sequence[index])) {
                return std::nullopt;
            }
        }

        return std::make_optional<OperatorSequence>(OperatorSequence::ConstructRawFlag{},
                                                    sequence, this->hash(sequence), *this);

    }

    SourceListBitset InflationContext::connected_sources(const OperatorSequence &seq) const {
        SourceListBitset output(this->total_inflated_sources);
        for (const auto op : seq) {
            const auto& opInfo = this->operator_info[op];
            const auto& observableInfo = this->inflated_observables[opInfo.observable];
            output |= observableInfo.variants[opInfo.variant].connected_sources;
        }

        return output;
    }

    SourceListBitset InflationContext::connected_sources(const oper_name_t op) const {
        assert(op < this->operator_count);
        const auto& opInfo = this->operator_info[op];
        const auto& observableInfo = this->inflated_observables[opInfo.observable];
        return observableInfo.variants[opInfo.variant].connected_sources;
    }

    bool InflationContext::can_be_simplified_as_moment(const OperatorSequence& input) const {
        // No aliasing without inflation, or for 0 or 1
        if (input.empty() || !this->can_have_aliases()) {
            return false;
        }

        SmallVector<oper_name_t, 4> next_free_source_variant(this->base_network.explicit_source_count(), 0);

        DynamicBitset<uint64_t, size_t, SmallVector<uint64_t, 1>> done_permutation{this->total_inflated_sources};

        // Go through operators looking for permutations
        for (const auto op : input) {
            const auto &opData = this->operator_info[op];
            const auto &observableInfo = this->inflated_observables[opData.observable];
            // Singleton operators are always canonical
            if (observableInfo.singleton) {
                continue;
            }

            // Is this the lowest variant possible at this point?
            const auto &variantInfo = observableInfo.variants[opData.variant];
            for (int s_index = observableInfo.sources.size()-1; s_index >= 0; --s_index) { // last-index-major

                const auto src_id = observableInfo.sources[s_index];
                assert(src_id < base_network.explicit_source_count());
                const auto src_variant = variantInfo.indices[s_index];
                const auto src_global = (this->inflation * src_id) + src_variant;

                if (!done_permutation.test(src_global)) {
                    const auto target_global = (this->inflation * src_id) + next_free_source_variant[src_id];
                    if (src_global != target_global) {
                        return true;
                    }

                    done_permutation.set(src_global);
                    ++next_free_source_variant[src_id];
                }
            }
        }

        return false;
    }


    OperatorSequence InflationContext::simplify_as_moment(OperatorSequence&& input) const {
        assert(this->can_have_aliases());
        // If 0, or I, or no inflation, then just pass through
        if (input.empty()) {
            return std::move(input);
        }

        SmallVector<oper_name_t, 4> next_free_source_variant(this->base_network.explicit_source_count(), 0);

        std::vector<oper_name_t>& permutation = get_permutation_scratch(this->total_inflated_sources);

        DynamicBitset<uint64_t, size_t, SmallVector<uint64_t, 1>> done_permutation{this->total_inflated_sources};
        bool non_trivial = false;

        // Go through operators looking for permutations
        for (const auto op : input) {
            const auto &opData = this->operator_info[op];
            const auto &observableInfo = this->inflated_observables[opData.observable];
            // Singleton operators are always canonical
            if (observableInfo.singleton) {
                continue;
            }

            // Is this the lowest variant possible at this point?
            const auto &variantInfo = observableInfo.variants[opData.variant];
            for (int s_index = observableInfo.sources.size()-1; s_index >= 0; --s_index) { // last-index-major
                const auto src_id = observableInfo.sources[s_index];
                assert(src_id < base_network.explicit_source_count());
                const auto src_variant = variantInfo.indices[s_index];
                const auto src_global = (this->inflation * src_id) + src_variant;

                if (!done_permutation.test(src_global)) {
                    const auto target_global = (this->inflation * src_id) + next_free_source_variant[src_id];
                    permutation[src_global] = target_global;
                    ++next_free_source_variant[src_id];
                    done_permutation.set(src_global);

                    non_trivial |= (src_global != target_global);
                }
            }
        }

        // Early exit if no permutations made
        if (!non_trivial) {
            return std::move(input);
        }

        // Permute operators
        sequence_storage_t permuted_operators;
        for (const oper_name_t op : input) {
            assert((op >= 0) && (op < this->operator_count));
            const auto& op_info = this->operator_info[op];
            const auto& obs_info = this->inflated_observables[op_info.observable];
            // Singleton operators are not permuted
            if (obs_info.singleton) {
                permuted_operators.emplace_back(op);
                continue;
            }

            const auto& variant_info = obs_info.variants[op_info.variant];
            const auto permuted_indices = this->base_network.permute_variant(this->inflation, obs_info.sources,
                                                                             permutation, variant_info.indices);
            const auto& permuted_variant = obs_info.variant(permuted_indices);

            permuted_operators.push_back(permuted_variant.operator_offset + op_info.outcome);
        }

        // If source-relabelling causes change in operator order, there could be further simplifications:
        if (!std::is_sorted(permuted_operators.cbegin(), permuted_operators.cend())) {
            return this->simplify_as_moment(OperatorSequence{std::move(permuted_operators), *this});
        }

        return OperatorSequence{std::move(permuted_operators), *this};
    }

    std::vector<OVOIndex>
    InflationContext::unflatten_outcome_index(const std::span<const OVIndex> input, oper_name_t outcome_number) const {
        // Empty input -> empty output
        if (input.empty()) {
            return {};
        }

        // Otherwise, first copy O/V indices, and bounds check
        std::vector<OVOIndex> output;
        output.reserve(input.size());
        size_t index = 0;
        for (const auto& ov : input) {
            if (ov.observable > this->inflated_observables.size()) {
                std::stringstream errSS;
                errSS << "Observable \"" << ov.observable << "\" at index " << index << " out of range.";
                throw errors::bad_observable{index, errSS.str()};
            }
            output.emplace_back(ov, 0);
            ++index;
        }

        // Now, we reverse iterate and deduce outcome number
        for (auto iter = output.rbegin(); iter != output.rend(); ++iter) {
            const auto max_outcomes = static_cast<oper_name_t>(
                    this->inflated_observables[iter->observable_variant.observable].outcomes);
            iter->outcome = static_cast<oper_name_t>(outcome_number % max_outcomes);
            outcome_number = static_cast<oper_name_t>(outcome_number / max_outcomes);
        }

        // Move output
        return output;
    }


    size_t InflationContext::flatten_outcome_index(std::span<const OVOIndex> input) const {
        size_t calculated_outcome_index = 0;
        size_t stride = 1;

        size_t input_index = input.size() - 1;
        for (auto indexIter = input.rbegin(); indexIter != input.rend(); ++indexIter) {
            const auto& index = *indexIter;
            const auto& ov = index.observable_variant;

            // Check observable
            if (index.observable_variant.observable >= this->inflated_observables.size()) {
                std::stringstream errSS;
                errSS << "Observable \"" << ov.observable << "\" at index " << input_index << " is out of range.";
                throw errors::bad_observable{input_index, errSS.str()};
            }
            const auto& observable = this->inflated_observables[ov.observable];

            // Check variant
            if (index.observable_variant.variant >= observable.variant_count) {
                std::stringstream errSS;
                errSS << "Variant \"" << ov.variant << "\" for observable \"" << ov.observable << "\" at index "
                      << input_index << " is out of range.";
                throw errors::bad_observable{input_index, errSS.str()};
            }

            // Check outcome
            if (index.outcome >= observable.outcomes) {
                std::stringstream errSS;
                errSS << "Outcome \"" << index.outcome << "\" for variant \""
                       << ov.variant << "\" of observable \"" << ov.observable << "\" at index "
                      << input_index << " is out of range.";
                throw errors::bad_observable{input_index, errSS.str()};
            }

            // Add to index
            calculated_outcome_index += stride * index.outcome;
            stride *= observable.outcomes;
            --input_index;
        }

        return calculated_outcome_index;
    }

    oper_name_t InflationContext::operator_number(oper_name_t observable, oper_name_t variant,
                                                  oper_name_t outcome) const noexcept {
       assert((observable >= 0) && (observable < this->inflated_observables.size()));
       assert((observable >= 0) && (observable < this->base_network.Observables().size()));
       const auto& observable_info = this->inflated_observables[observable];
       assert((variant >= 0) && (variant < observable_info.variant_count));
       const auto& base_observable = this->base_network.Observables();
       return static_cast<oper_name_t>(observable_info.operator_offset
                + (variant * static_cast<oper_name_t>(this->base_network.Observables()[observable].operators()))
                + outcome);
   }

    oper_name_t InflationContext::obs_variant_to_index(const oper_name_t observable, const oper_name_t variant) const {
        assert((observable >= 0) && (observable < this->inflated_observables.size()));
        const auto& observable_info = this->inflated_observables[observable];
        assert((variant >= 0) && (variant < observable_info.variant_count));
        return static_cast<oper_name_t>(observable_info.variant_offset + variant);
    }

    OVIndex
    InflationContext::index_to_obs_variant(const oper_name_t global_variant_index) const {
        assert((global_variant_index >= 0) && (global_variant_index < this->global_variant_indices.size()));
        return this->global_variant_indices[global_variant_index];
    }

    void InflationContext::format_sequence(ContextualOS& os, const OperatorSequence &seq) const {
        assert(dynamic_cast<const InflationContext*>(&os.context) == this);

        if (seq.zero()) {
            os.os << "0";
            return;
        }
        if (seq.empty()) {
            os.os << "1";
            return;
        }

        if (seq.negated()) {
            os.os << "-";
        }

        if (os.format_info.show_braces) {
            os.os << "<";
        }

        AlphabeticNamer obsNamer{true};

        const bool needs_comma = this->inflation > 9;
        const bool needs_braces = std::any_of(this->Observables().cbegin(), this->Observables().cend(),
                                              [](const auto& obs) { return obs.outcomes > 2; });
        bool one_operator = false;
        for (const auto& oper : seq) {
            if (one_operator) {
                os.os << ";";
            } else {
                one_operator = true;
            }

            if (oper >= this->operator_info.size()) {
                os.os << "[UNK:" << oper << "]";
            } else {
                const auto &extraInfo = this->operator_info[oper];
                const auto &obsInfo = this->inflated_observables[extraInfo.observable];

                os.os << obsNamer(extraInfo.observable);
                if (obsInfo.outcomes > 2) {
                    os.os << extraInfo.outcome;
                }
                // Give indices, if inflated
                if (this->inflation > 1) {
                    const auto& infIndices = obsInfo.variants[extraInfo.variant].indices;
                    bool done_one = false;
                    if (needs_braces) {
                        os.os << "[";
                    }
                    for (auto infIndex : infIndices){
                        if (needs_comma && done_one) {
                            os.os << ",";
                        } else {
                            done_one = true;
                        }
                        os.os << infIndex;
                    }
                    if (needs_braces) {
                        os.os << "]";
                    }
                }
            }
        }

        if (os.format_info.show_braces) {
            os.os << ">";
        }
    }

    std::string InflationContext::format_sequence(const std::vector<OVOIndex> &indices) const {
        if (indices.empty()) {
            return "1";
        }

        std::stringstream ss;
        AlphabeticNamer obsNamer{true};

        const bool needs_comma = this->inflation > 9;
        const bool needs_braces = std::any_of(this->Observables().cbegin(), this->Observables().cend(),
                                              [](const auto& obs) { return obs.outcomes > 2; });
        bool one_operator = false;
        for (const auto& ovo : indices) {
            if (one_operator) {
                ss << ";";
            } else {
                one_operator = true;
            }

            // Check valid observable
            if (ovo.observable_variant.observable >= this->inflated_observables.size()) {
                ss << "[UNK: " << ovo.observable_variant.observable << ", "
                               << ovo.observable_variant.variant << ", "
                               << ovo.outcome << "]";
            } else {

                // Name observable
                const auto &obsInfo = this->inflated_observables[ovo.observable_variant.observable];
                ss << obsNamer(ovo.observable_variant.observable);

                // Give indices, if inflated
                if (this->inflation > 1) {
                    // Check variant index
                    if (ovo.observable_variant.variant >= obsInfo.variant_count) {
                        ss << "[UNK-VAR: " << ovo.observable_variant.variant << "]";

                    } else {
                        const auto &infIndices = obsInfo.variants[ovo.observable_variant.variant].indices;

                        bool done_one = false;
                        if (needs_braces) {
                            ss << "[";
                        }
                        for (auto infIndex: infIndices) {
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

            // Write output number
            ss << "." << ovo.outcome;

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

    std::unique_ptr<OperatorSequenceGenerator> InflationContext::new_osg(size_t word_length) const {
        return std::make_unique<InflationOperatorSequenceGenerator>(*this, word_length);
    }



}
