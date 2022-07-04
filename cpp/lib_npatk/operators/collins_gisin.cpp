/**
 * collins_gisin.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "collins_gisin.h"
#include "multi_mmt_iterator.h"
#include "moment_matrix.h"
#include "utilities/combinations.h"

namespace NPATK {

    CollinsGisinForm::CollinsGisinForm(const MomentMatrix& momentMatrix, size_t level)
        : Level{level}, indices(momentMatrix.context.measurement_count(), level) {

        const Context& context = momentMatrix.context;

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (momentMatrix.UniqueSequences.size() < 2) {
            throw errors::cg_form_error("Zero and One should be defined in MomentMatrix.");
        }
        const auto& oneSeq = momentMatrix.UniqueSequences[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::cg_form_error("Identity symbol was improperly defined in MomentMatrix.");
        }

        // Base level points to identity element symbol
        this->indices.set({0,1});
        this->data.push_back(1);
        size_t index_counter = 1;

        // For each level,
        for (size_t current_level = 1; current_level < (Level+1); ++current_level) {

            // Iterate through party combinations:
            CombinationIndexIterator index_combo{context.Parties.size(), current_level};
            while (!index_combo.done()) {
                // Choose parties from indices
                const auto& partyIndices = *index_combo;
                assert(partyIndices.size() == current_level);

                bool all_good = true;
                MultiMmtIterator::party_list_t  pmiStack;
                for (size_t i = 0; i < current_level; ++i) {
                    // If party has no measurements, skip combination
                    if (context.Parties[partyIndices[i]].Measurements.empty()) {
                        all_good = false;
                        break;
                    }
                    pmiStack.emplace_back(&(context.Parties[partyIndices[i]]));
                }
                // Skip
                if (!all_good) {
                    ++index_combo;
                    continue;
                }

                // Iterate through mmts of chosen parties
                MultiMmtIterator multiMmtIterator{momentMatrix.context, std::move(pmiStack)};
                while (!multiMmtIterator.done()) {
                    auto mmtIndices = multiMmtIterator.indices();
                    size_t num_operators = multiMmtIterator.count_operators();
                    if (num_operators == 0) {
                        ++multiMmtIterator;
                        continue;
                    }

                    this->data.reserve(this->data.size() + num_operators);

                    // Now, iterate over every operator sequence
                    auto opIter = multiMmtIterator.begin_operators();
                    const auto opIterEnd = multiMmtIterator.end_operators();
                    while (opIter != opIterEnd) {
                        // Find symbol for operator sequence
                        auto symbol_loc = momentMatrix.UniqueSequences.where(*opIter);
                        if (symbol_loc == nullptr) {
                            throw errors::cg_form_error{"Could not find expected symbol in MomentMatrix."};
                        }
                        this->data.emplace_back(symbol_loc->Id());

                        ++opIter;
                    }

                    // Add mmt
                    this->indices.set(multiMmtIterator.global_indices(), {index_counter, index_counter + num_operators});
                    index_counter += num_operators;
                    assert(this->data.size() == index_counter);

                    // Advance to next measurement
                    ++multiMmtIterator;
                }

                // Next combination of party indices
                ++index_combo;
            }
        }
    }

    std::span<const symbol_name_t> CollinsGisinForm::get_global(std::span<const size_t> mmtIndices) const {
        auto [first, last] = this->indices.access(mmtIndices);
        if ((first < 0) || (first >= last)) {
            return {this->data.begin(), 0};
        }
        assert(last <= this->data.size());
        return {this->data.begin() + first, static_cast<size_t>(last - first)};
    }

}