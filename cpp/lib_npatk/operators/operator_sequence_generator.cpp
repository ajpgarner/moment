/*
 * unique_operator_strings.cpp
 *
 * (c) 2022-2022 Austrian Academy of Sciences.
 */
#include "operator_sequence_generator.h"
#include "multi_operator_iterator.h"

#include <map>
#include <algorithm>

namespace NPATK {
    OperatorSequenceGenerator::OperatorSequenceGenerator(const Context &operatorContext, size_t chain_length)
        : context(operatorContext), sequence_length(chain_length) {
        std::map<size_t, OperatorSequence> build_set;

        // For all lengths, even 1, we include ID operator.
        build_set.emplace(1, OperatorSequence::Identity(context));

        // Iterate through various generators...
        for (size_t sub_length = 1; sub_length <= sequence_length; ++sub_length) {
            for (auto seq: detail::MultiOperatorRange{context, sub_length}) {
                if (seq.zero()) {
                    continue;
                }
                size_t hash = context.hash(seq);
                build_set.emplace(hash, std::move(seq));
            }
        }


        this->unique_sequences.reserve(build_set.size());
        std::transform(build_set.begin(), build_set.end(), std::back_inserter(this->unique_sequences),
                       [](auto& swh) -> OperatorSequence&& { return std::move(swh.second); });

    }


    OperatorSequenceGenerator OperatorSequenceGenerator::conjugate() const {
        std::vector<OperatorSequence> conjList{};
        conjList.reserve(this->unique_sequences.size());
        for (const auto& seq : this->unique_sequences) {
            conjList.emplace_back(seq.conjugate());
        }

        return OperatorSequenceGenerator{this->context, this->sequence_length, std::move(conjList)};
    }

}