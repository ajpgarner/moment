/**
 * remapper.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "remapper.h"

#include "integer_types.h"

#include "matrix/operator_sequence_generator.h"

#include "scenarios/context.h"
#include "scenarios/operator_sequence.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment {

    Remapper::Remapper(const Context &context, const size_t max_word_length)
        : context{context} {

        // First, reverse OSG to get map from hash to order.
        OperatorSequenceGenerator osg(context,0, max_word_length);
        std::map<size_t, size_t> hash_to_index;
        size_t osg_index = 0;
        for (const auto& seq : osg) {
            hash_to_index.emplace_hint(
                    hash_to_index.end(),
                    std::make_pair(seq.hash(), osg_index)
                );
            ++osg_index;
        }

        // XXX: '1+op_count' will fail on contexts with single character rewrite equivalences (e.g. "b = a")

        // Now, iterate through raw indices to make dense map from potential words to canonical index
        auto op_count = context.size();
        for (const auto& raw_vec : MultiDimensionalIndexRange{std::vector<size_t>(max_word_length, 1+op_count)}) {
            sequence_storage_t seq_vec;
            for (const auto x : raw_vec) {
                if (x > 0) {
                    seq_vec.emplace_back(x - 1);
                }
            }
            OperatorSequence op_seq{std::move(seq_vec), context};

            auto remap_iter = hash_to_index.find(op_seq.hash());
            assert(remap_iter != hash_to_index.cend());
            this->remap.emplace_back(remap_iter->second);
        }
    }
}