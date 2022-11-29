/**
 * explicit_symbol.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "explicit_symbols.h"

#include "locality_context.h"
#include "joint_measurement_iterator.h"
#include "locality_matrix_system.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

#include "operators/matrix/symbol_table.h"
#include "utilities/combinations.h"

namespace NPATK {
    namespace {
        std::vector<size_t> makeOpCounts(const LocalityContext& context) {
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

        std::vector<size_t> makeOpCounts(const InflationContext& context) {
            std::vector<size_t> output;
            output.reserve(context.Observables().size());
            for (const auto& o : context.Observables()) {
                output.emplace_back(o.outcomes-1);
            }
            return output;
        }
    }

    ExplicitSymbolIndex::ExplicitSymbolIndex(const LocalityMatrixSystem& matrixSystem, const size_t level)
        : Level{level},
          indices{matrixSystem.localityContext, level},
          OperatorCounts{makeOpCounts(matrixSystem.localityContext)} {

        const auto& context = matrixSystem.localityContext;
        const SymbolTable& symbols = matrixSystem.Symbols();

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (symbols.size() < 2) {
            throw errors::cg_form_error("Zero and One should be defined.");
        }
        const auto& oneSeq = symbols[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::cg_form_error("Identity symbol was improperly defined.");
        }

        // Base level points to identity element symbol
        this->indices.set({0,1});
        this->data.push_back({1, matrixSystem.Symbols().to_basis(1).first});
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
                JointMeasurementIterator multiMmtIterator{context, std::move(pmiStack)};
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
                        auto symbol_loc = symbols.where(*opIter);
                        if (symbol_loc == nullptr) {
                            throw errors::cg_form_error{"Could not find expected symbol in MomentMatrix."};
                        }
                        this->data.emplace_back(ExplicitSymbolEntry{symbol_loc->Id(), symbol_loc->basis_key().first});

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

    ExplicitSymbolIndex::ExplicitSymbolIndex(const InflationMatrixSystem& matrixSystem, const size_t level)
        : Level{level},
          indices{matrixSystem.InflationContext(), level},
          OperatorCounts{makeOpCounts(matrixSystem.InflationContext())} {

        const auto& context = matrixSystem.InflationContext();
        const auto& observables = context.Observables();
        const SymbolTable& symbols = matrixSystem.Symbols();

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (symbols.size() < 2) {
            throw errors::cg_form_error("Zero and One should be defined.");
        }
        const auto& oneSeq = symbols[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::cg_form_error("Identity symbol was improperly defined.");
        }

        // Base level points to identity element symbol
        this->indices.set({0,1});
        this->data.push_back({1, matrixSystem.Symbols().to_basis(1).first});
        size_t index_counter = 1;

        // For each level,
        for (size_t current_level = 1; current_level < (Level+1); ++current_level) {

            // Iterate through party combinations:
            CombinationIndexIterator index_combo{observables.size(), current_level};
            while (!index_combo.done()) {
                // Choose parties from indices
                const auto& partyIndices = *index_combo;
                assert(partyIndices.size() == current_level);

                // Count operators associated with chosen parties
                std::vector<size_t> opers_per_observable;
                opers_per_observable.reserve(partyIndices.size());
                for (const auto party : partyIndices) {
                    opers_per_observable.emplace_back(observables[party].outcomes - 1);
                }
                const auto num_operators = std::reduce(opers_per_observable.cbegin(), opers_per_observable.cend(),
                                                       static_cast<size_t>(1), std::multiplies{});

                this->data.reserve(this->data.size() + num_operators);

                // Now, iterate over every operator sequence
                MultiDimensionalIndexIterator opIndicesIter{opers_per_observable, false};
                const MultiDimensionalIndexIterator opIndicesIterEnd{opers_per_observable, true};
                while (opIndicesIter != opIndicesIterEnd) {
                    // Find symbol for operator sequence
                    auto opIndices = *opIndicesIter;
                    std::vector<oper_name_t> op_str;
                    op_str.reserve(opIndices.size());
                    for (size_t i = 0; i < opIndices.size(); ++i) {
                        op_str.emplace_back(observables[partyIndices[i]].operator_offset + opIndices[i]);
                    }

                    OperatorSequence opSeq{std::move(op_str), context};
                    auto symbol_loc = symbols.where(opSeq);
                    if (symbol_loc == nullptr) {
                        throw errors::cg_form_error{"Could not find expected symbol in MomentMatrix."};
                    }
                    this->data.emplace_back(ExplicitSymbolEntry{symbol_loc->Id(), symbol_loc->basis_key().first});

                    ++opIndicesIter;
                }

                // Add index
                this->indices.set(partyIndices, {index_counter, index_counter + num_operators});
                index_counter += num_operators;
                assert(this->data.size() == index_counter);

                // Next combination of party indices
                ++index_combo;
            }
        }
    }



    std::span<const ExplicitSymbolEntry> ExplicitSymbolIndex::get(std::span<const size_t> mmtIndices) const {
        auto [first, last] = this->indices.access(mmtIndices);
        if ((first < 0) || (first >= last)) {
            return {this->data.begin(), 0};
        }
        assert(last <= this->data.size());
        return {this->data.begin() + first, static_cast<size_t>(last - first)};
    }

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