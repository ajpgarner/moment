/**
 * explicit_symbols.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "explicit_symbols.h"

#include "utilities/multi_dimensional_index_iterator.h"

#include <algorithm>

namespace Moment {
    std::vector<ExplicitSymbolEntry>
    ExplicitSymbolIndex::get(const std::span<const size_t> mmtIndices,
                             const std::span<const oper_name_t> fixedOutcomes) const {
        assert(mmtIndices.size() == fixedOutcomes.size());
        // Number of outcomes to copy; also which is fixed
        std::vector<size_t> iterating_indices;
        std::vector<bool> iterates;
        std::vector<size_t> iteratingSizes;

        // Examine which indices are fixed, and which iterate
        size_t total_outcomes = 1;
        for (size_t i = 0; i < mmtIndices.size(); ++i) {
            if (fixedOutcomes[i] == -1) {
                const auto op_count = this->operator_counts[mmtIndices[i]];
                iterating_indices.push_back(mmtIndices[i]);
                iteratingSizes.push_back(op_count);
                total_outcomes *= op_count;
                iterates.push_back(true);
            } else {
                iterates.push_back(false);
            }
        }

        const auto num_iterating_indices = iterating_indices.size();

        // Get span to measurement
        auto fullMmtSpan = this->get(mmtIndices);

        // If all indices iterate, just copy output, and exit early
        if (num_iterating_indices == mmtIndices.size()) {
            return {fullMmtSpan.begin(), fullMmtSpan.end()};
        }

        // Calculate strides for free indices, and offset for fixed ones.
        size_t the_offset = 0;
        size_t current_stride = 1;
        std::vector<size_t> stride;
        for (size_t m = 0; m < mmtIndices.size(); ++m) {
            const size_t invM = mmtIndices.size() - m - 1;

            if (iterates[invM]) {
                stride.push_back(current_stride);
            } else {
                assert (fixedOutcomes[invM] != -1);
                the_offset += (current_stride * fixedOutcomes[invM]);
            }
            current_stride *= this->operator_counts[mmtIndices[invM]];
        }

        // No indices iterate, so we retrieve just one value
        if (num_iterating_indices == 0) {
            return {fullMmtSpan[the_offset]};
        }

        // Otherwise, we have the more complex case:
        std::vector<ExplicitSymbolEntry> output;
        output.reserve(total_outcomes);

        // Make iterator over free indices
        std::reverse(iteratingSizes.begin(), iteratingSizes.end());
        MultiDimensionalIndexIterator freeOutcomeIndexIter{std::move(iteratingSizes)};

        // Blit values we care about
        while (!freeOutcomeIndexIter.done()) {
            size_t the_index = the_offset;
            for (size_t i = 0 ; i < num_iterating_indices; ++i) {
                the_index += freeOutcomeIndexIter[i] * stride[i];
            }
            output.push_back(fullMmtSpan[the_index]);

            // Onto next
            ++freeOutcomeIndexIter;
        }

        return output; // NRVO
    }

}
