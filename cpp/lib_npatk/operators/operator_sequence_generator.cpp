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

    namespace {
        struct SequenceHasher {
            const OperatorCollection& context;
            explicit SequenceHasher(const OperatorCollection& context) : context{context} { }
            constexpr size_t operator()(const OperatorSequence& sequence) {
                size_t hash = 0;
                size_t multiplier = 1;

                for (size_t n = 0; n < sequence.size(); ++n) {
                    const auto& oper = sequence[sequence.size()-n-1];
                    size_t global_index = context.Parties[oper.party.id].offset() + oper.id;
                    hash += (global_index * multiplier);
                    multiplier *= context.size();
                }
                return hash;
            }
        };
    }

    OperatorSequenceGenerator::OperatorSequenceGenerator(const OperatorCollection &operatorContext, size_t chain_length)
        : context(operatorContext), sequence_length(chain_length) {
        std::map<size_t, OperatorSequence> build_set;

        detail::MultiOperatorIterator combo_iter{context, sequence_length};
        const detail::MultiOperatorIterator combo_iter_end = detail::MultiOperatorIterator::end_of(context, sequence_length);
        SequenceHasher hasher{context};

        while (combo_iter != combo_iter_end) {
            auto seq = *combo_iter;
            size_t hash = hasher(seq);
            build_set.emplace(hash, std::move(seq));
            ++combo_iter;
        }

        this->unique_sequences.reserve(build_set.size());
        std::transform(build_set.begin(), build_set.end(), std::back_inserter(this->unique_sequences),
                       [](auto& swh) -> OperatorSequence&& { return std::move(swh.second); });

    }
}