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

        for (auto seq : detail::MultiOperatorRange{context, sequence_length}) {
            if (seq.zero()) {
                continue;
            }
            size_t hash = context.hash(seq);
            build_set.emplace(hash, std::move(seq));
        }

        this->unique_sequences.reserve(build_set.size());
        std::transform(build_set.begin(), build_set.end(), std::back_inserter(this->unique_sequences),
                       [](auto& swh) -> OperatorSequence&& { return std::move(swh.second); });

    }

    OperatorSequenceGenerator::OperatorSequenceGenerator(const Context& operatorContext,
                                                         size_t chain_length,
                                                         std::vector<OperatorSequence> &&seq)
            : context(operatorContext), sequence_length(chain_length) {
        this->unique_sequences.swap(seq);
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