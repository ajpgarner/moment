/**
 * collins_gisin.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "collins_gisin.h"
#include "joint_measurement_iterator.h"
#include "matrix/moment_matrix.h"
#include "utilities/combinations.h"

namespace NPATK {
    namespace {
        std::vector<size_t> makeOpCounts(const Context& context) {
            std::vector<size_t> output;
            output.reserve(context.measurement_count());
            size_t i = 0;
            for (const auto& p : context.Parties) {
                for (const auto& m : p.Measurements) {
                    assert(m.Index().global_mmt == i);
                    output.push_back(m.num_operators());
                    ++i;
                }
            }
            assert(i == context.measurement_count());
            return output;
        }
    }

    CollinsGisinForm::CollinsGisinForm(const MomentMatrix& momentMatrix, const size_t level)
        : Level{level},
          indices{momentMatrix.context, level},
          OperatorCounts(makeOpCounts(momentMatrix.context)) {

        const Context& context = momentMatrix.context;

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (momentMatrix.Symbols.size() < 2) {
            throw errors::cg_form_error("Zero and One should be defined in MomentMatrix.");
        }
        const auto& oneSeq = momentMatrix.Symbols[1];
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
                JointMeasurementIterator::party_list_t  pmiStack;
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
                JointMeasurementIterator multiMmtIterator{momentMatrix.context, std::move(pmiStack)};
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
                        auto symbol_loc = momentMatrix.Symbols.where(*opIter);
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

    std::span<const symbol_name_t> CollinsGisinForm::get(std::span<const size_t> mmtIndices) const {
        auto [first, last] = this->indices.access(mmtIndices);
        if ((first < 0) || (first >= last)) {
            return {this->data.begin(), 0};
        }
        assert(last <= this->data.size());
        return {this->data.begin() + first, static_cast<size_t>(last - first)};
    }

    std::vector<symbol_name_t>
    CollinsGisinForm::get(const std::span<const size_t> mmtIndices,
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
                const auto op_count = this->OperatorCounts[mmtIndices[i]];
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
            current_stride *= this->OperatorCounts[mmtIndices[invM]];
        }

        // No indices iterate, so we retrieve just one value
        if (num_iterating_indices == 0) {
            return {fullMmtSpan[the_offset]};
        }

        // Otherwise, we have the more complex case:
        std::vector<symbol_name_t> output;
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