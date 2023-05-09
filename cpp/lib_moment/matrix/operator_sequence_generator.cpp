/*
 * operator_sequence_generator.cpp
 *
 * @copyright Copyright (c) 2022-2022 Austrian Academy of Sciences.
 * @author Andrew J. P. Garner
 */
#include "operator_sequence_generator.h"

#include "scenarios/multi_operator_iterator.h"

#include <algorithm>
#include <limits>
#include <map>

namespace Moment {

    std::vector<OperatorSequence>
    OperatorSequenceGenerator::build_generic_sequences(const Context &context,
                                                       size_t min_sequence_length, size_t max_sequence_length) {

        std::map<size_t, OperatorSequence> build_set;

        // If minimum length, include identity...
        if (0 == min_sequence_length) {
            build_set.emplace(1, OperatorSequence::Identity(context));
        }

        // Iterate through various generators...
        for (size_t sub_length = min_sequence_length; sub_length <= max_sequence_length; ++sub_length) {
            for (auto seq : MultiOperatorRange{context, sub_length}) {
                if (seq.zero()) {
                    continue;
                }
                size_t hash = context.hash(seq);
                build_set.emplace(hash, std::move(seq));
            }
        }

        // Copy to output
        std::vector<OperatorSequence> output;
        output.reserve(build_set.size());
        std::transform(build_set.begin(), build_set.end(), std::back_inserter(output),
                       [](auto& swh) -> OperatorSequence&& { return std::move(swh.second); });
        return output;
    }



    OperatorSequenceGenerator OperatorSequenceGenerator::conjugate() const {
        std::vector<OperatorSequence> conjList{};
        conjList.reserve(this->unique_sequences.size());
        size_t shortest = std::numeric_limits<size_t>::max();
        size_t longest = 0;

        for (const auto& seq : this->unique_sequences) {
            auto seqConj = seq.conjugate();
            size_t len = seqConj.size();
            if (len > longest) {
                longest = len;
            }
            if (len < shortest) {
                shortest = len;
            }

            conjList.emplace_back(std::move(seqConj));
        }

        return OperatorSequenceGenerator{this->context, shortest, longest, std::move(conjList)};
    }



}