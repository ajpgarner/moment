/*
 * operator_sequence_generator.cpp
 *
 * @copyright Copyright (c) 2022-2022 Austrian Academy of Sciences.
 * @author Andrew J. P. Garner
 */
#include "operator_sequence_generator.h"

#include "multi_operator_iterator.h"

#include <algorithm>
#include <limits>
#include <map>

namespace Moment {

    std::vector<OperatorSequence>
    OperatorSequenceGenerator::build_generic_sequences(const Context &context, size_t max_sequence_length) {

        std::vector<OperatorSequence> output;

        // Always include identity
        output.emplace_back(OperatorSequence::Identity(context));

        // Iterate through various generators...
        for (size_t sub_length = 1; sub_length <= max_sequence_length; ++sub_length) {
            const auto oper_iter_end = MultiOperatorIterator::end_of(context, sub_length);
            for (auto oper_iter = MultiOperatorIterator{context, sub_length}; oper_iter != oper_iter_end; ++oper_iter) {

                auto seq = context.get_if_canonical(oper_iter.raw());
                if (seq.has_value()) {
                    output.emplace_back(std::move(seq.value()));
                }
            }
        }

        return output;
    }


    OperatorSequenceGenerator OperatorSequenceGenerator::conjugate() const {
        std::vector<OperatorSequence> conjList{};
        conjList.reserve(this->unique_sequences.size());
        size_t longest = 0;

        for (const auto& seq : this->unique_sequences) {
            auto seqConj = seq.conjugate();
            size_t len = seqConj.size();
            if (len > longest) {
                longest = len;
            }

            conjList.emplace_back(std::move(seqConj));
        }

        return OperatorSequenceGenerator{this->context, longest, std::move(conjList)};
    }



}