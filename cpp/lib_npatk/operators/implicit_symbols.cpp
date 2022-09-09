/**
 * implicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "context.h"
#include "collins_gisin.h"
#include "implicit_symbols.h"
#include "matrix/matrix_system.h"
#include "matrix/symbol_table.h"

#include "joint_measurement_iterator.h"

#include "utilities/combinations.h"

#include <algorithm>

namespace NPATK {

    ImplicitSymbols::ImplicitSymbols(const MatrixSystem &ms)
        : symbols{ms.Symbols()},
          context{ms.Context()},
          cgForm{ms.CollinsGisin()},
          MaxSequenceLength{ms.MaxRealSequenceLength()},
          indices{ms.Context(), ms.MaxRealSequenceLength()} {

        size_t index_cursor = 0;
        this->generateLevelZero(index_cursor);

        if (MaxSequenceLength >= 1) {
            this->generateLevelOne(index_cursor);
        }

        for (size_t level = 2; level <= MaxSequenceLength; ++level) {
            this->generateMoreLevels(level, index_cursor);
        }
    }

    size_t ImplicitSymbols::generateLevelZero(size_t& index_cursor) {
        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (symbols.size() < 2) {
            throw errors::bad_implicit_symbol("Zero and One should be defined in MomentMatrix.");
        }
        const auto& oneSeq = symbols[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::bad_implicit_symbol("Identity symbol was improperly defined in MomentMatrix.");
        }

        // Construct level 0 with just normalization
        this->tableData.emplace_back(1, SymbolCombo{{1, 1.0}});
        this->indices.set({0, 1});
        ++index_cursor;

        return 1;
    }

    size_t ImplicitSymbols::generateLevelOne(size_t& index_cursor) {
        ptrdiff_t level_one_count = 0;

        // Iterate through parties & measurements, also here perform some basic checks
        for (const auto& party : context.Parties) {
            for (const auto& mmt : party.Measurements) {

                const size_t mmt_index_start = index_cursor;


                // Only complete measurements can be inferred in this way...
                if (!mmt.complete) {
                    [[unlikely]]
                    throw errors::bad_implicit_symbol(std::string("Correlation table can only be generated when")
                                                                  + " all measurements are complete.");
                }

                // PRECONDITION: Measurement has one more outcome than defined operators...
                if (mmt.num_outcomes != (mmt.num_operators()+1)) {
                    [[unlikely]]
                    throw errors::bad_implicit_symbol(
                            "Measurement should have one more outcome than explicit operators.");
                }

                // Get explicit outcomes
                auto mmtSymb = this->cgForm.get({static_cast<size_t>(mmt.Index().global_mmt)});
                if (mmtSymb.size() != mmt.num_operators()) {
                    throw errors::bad_implicit_symbol("Could not find measurement in Collins-Gisin table.");
                }

                // Explicit outcomes:
                SymbolCombo::data_t finalOutcome{{1, 1.0}};
                for (uint32_t outcome = 0; outcome < mmt.num_operators(); ++outcome) {
                    // Read symbol from Collins-Gisin object
                    const auto symbol_id = mmtSymb[outcome];
                    this->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
                    finalOutcome.push_back({symbol_id, -1.0});
                    ++level_one_count;
                }

                // Add final measurement outcome, which is linear sum of remaining outcomes
                this->tableData.emplace_back(-1, LinearCombo(std::move(finalOutcome)));
                ++level_one_count;

                // Make index for measurement
                std::vector<size_t> index{static_cast<size_t>(mmt.Index().global_mmt)};
                this->indices.set(index, {mmt_index_start, mmt_index_start + mmt.num_outcomes});
                index_cursor += mmt.num_outcomes;
            }
        }
        assert(index_cursor == this->tableData.size());
        return level_one_count;
    }



    size_t ImplicitSymbols::generateMoreLevels(const size_t level, size_t& index_cursor) {
        assert(level <= this->MaxSequenceLength);

        const size_t init_cursor  = index_cursor;

        // Iterate through party combinations:
        CombinationIndexIterator index_combo{this->context.Parties.size(), level};
        while (!index_combo.done()) {
            // Choose parties from indices
            const auto& partyIndices = *index_combo;
            assert(partyIndices.size() == level);
            JointMeasurementIterator::party_list_t pmiStack;
            for (size_t i = 0; i < level; ++i) {
                pmiStack.emplace_back(&(this->context.Parties[partyIndices[i]]));
            }

            // Iterate through measurements of chosen parties
            JointMeasurementIterator partyStack(this->context, std::move(pmiStack));
            while (!partyStack.done()) {
                this->generateFromCurrentStack(partyStack, index_cursor);
                ++partyStack;
            }

            // Next combination of party indices
            ++index_combo;
        }

        assert(index_cursor >= init_cursor);
        return index_cursor - init_cursor;
    }


    size_t ImplicitSymbols::generateFromCurrentStack(const JointMeasurementIterator& stack,
                                                     size_t& index_cursor) {
        const size_t level = stack.count_indices();
        const size_t num_outcomes = stack.count_outcomes();

        for (auto outcomeIter = stack.begin_outcomes(), outcomeIterEnd = stack.end_outcomes();
                    outcomeIter != outcomeIterEnd; ++outcomeIter) {
            const size_t num_implicit = outcomeIter.implicit_count();
            if (num_implicit == 0) {
                // No implicit operators, just put down the explicit operator
                const auto implicit_full_opers = this->cgForm.get(stack.global_indices());
                assert(implicit_full_opers.size() == stack.count_operators());
                assert(outcomeIter.explicit_outcome_index() < implicit_full_opers.size());
                const auto symbol_id = implicit_full_opers[outcomeIter.explicit_outcome_index()];
                this->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
            } else {
                SymbolCombo::data_t symbolComboData;
                double the_sign = (num_implicit % 2 == 0) ? +1. : -1.;
                for (size_t missing_index = num_implicit; missing_index > 0; --missing_index) {
                    PartitionIterator partitions{num_implicit, missing_index};
                    while (!partitions.done()) {

                        // Build measurement index query:
                        std::vector<size_t> lookupIndices;
                        std::vector<symbol_name_t> outcomeIndices;
                        size_t mNum = 0;
                        for (size_t i = 0; i < level; ++i) {
                            if (outcomeIter.implicit(i)) {
                                // Implicit measurement: either push -1, or skip
                                if (partitions.bits(mNum)) {
                                    lookupIndices.push_back(stack.global_indices()[i]);
                                    outcomeIndices.push_back(-1);
                                }
                                ++mNum;
                            } else {
                                // Push explicit measurement
                                lookupIndices.push_back(stack.global_indices()[i]);
                                outcomeIndices.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
                            }
                        }

                        // Look up, and copy with sign
                        const auto symbolsSpan = this->cgForm.get(lookupIndices, outcomeIndices);
                        for (auto symb : symbolsSpan) {
                            symbolComboData.emplace_back(symb, the_sign);
                        }
                        ++partitions;
                    }
                    the_sign = -the_sign;
                }

                // Finally, find the "Normalization" term
                assert(the_sign == 1); // If correctly alternating, normalization should be positive always.
                std::vector<size_t> normIndices;
                std::vector<symbol_name_t> normOutcomes;
                for (size_t i = 0; i < stack.count_indices(); ++i) {
                    if (!outcomeIter.implicit(i)) {
                        normIndices.push_back(stack.global_indices()[i]);
                        normOutcomes.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
                    }
                }
                auto normMmtSpan = this->cgForm.get(normIndices, normOutcomes);
                assert(normMmtSpan.size() == 1);
                symbolComboData.emplace_back(normMmtSpan[0], the_sign);

                // Add constructed representation to data table
                this->tableData.emplace_back(-1, SymbolCombo{std::move(symbolComboData)});
            }
        }

        // Add index for this mmt
        this->indices.set(stack.global_indices(), {index_cursor, index_cursor + num_outcomes});
        index_cursor += num_outcomes;
        assert(index_cursor == this->tableData.size());
        return num_outcomes;
    }

    std::span<const PMODefinition> ImplicitSymbols::get(const std::span<const size_t> mmtIndex) const {
        if (mmtIndex.size() > this->MaxSequenceLength) {
            throw errors::bad_implicit_symbol("Cannot look up sequences longer than the max sequence length.");
        }

        auto [first, last] = this->indices.access(mmtIndex);
        if ((first < 0) || (first >= last)) {
            return {tableData.begin(), 0};
        }
        assert(last <= tableData.size());
        return {tableData.begin() + first, static_cast<size_t>(last - first)};
    }

    const PMODefinition& ImplicitSymbols::get(std::span<const PMOIndex> lookup_indices) const {
        // First, PMO index to global index to look up by measurement
        std::vector<size_t> globalIndices;
        for (const auto& index : lookup_indices) {
            globalIndices.emplace_back(context.get_global_mmt_index(index));
        }

        // Get span of data for this measurement
        auto defsForMmt = this->get(globalIndices);
        if (defsForMmt.empty()) {
            throw errors::bad_implicit_symbol("Could not find implicit symbols for supplied measurement");
        }

        // If {} requested, return only entry...
        if (lookup_indices.empty()) {
            return defsForMmt[0];
        }

        // Otherwise, now find the requested outcome
        size_t the_offset = 0;
        size_t current_stride = 1;
        for (size_t i = 0; i < lookup_indices.size(); ++i) {
            const size_t inv_index = lookup_indices.size() - i - 1;
            const auto& index = lookup_indices[inv_index];
            the_offset += (current_stride * index.outcome);
            current_stride *= this->context.Parties[index.party].Measurements[index.mmt].num_outcomes;
        }
        assert(the_offset < defsForMmt.size());
        assert(current_stride == defsForMmt.size());

        return defsForMmt[the_offset];

    }


}