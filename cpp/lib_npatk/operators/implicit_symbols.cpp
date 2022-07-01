/**
 * implicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "implicit_symbols.h"

#include "multi_mmt_iterator.h"

#include "utilities/combinations.h"

#include <algorithm>

namespace NPATK {


    ImplicitSymbols::ImplicitSymbols(const MomentMatrix &mm)
        : momentMatrix{mm},
          context{mm.context},
          cgForm{mm.CollinsGisin()},
          MaxSequenceLength{mm.max_probability_length} {

        this->probabilityTable = std::make_unique<ProbabilityTable>(context.measurement_count(), MaxSequenceLength);
        assert(this->probabilityTable);

        size_t index_cursor = 0;
        this->generateLevelZero(index_cursor);

        if (MaxSequenceLength >= 1) {
            this->generateLevelOne(index_cursor);
        }

        for (size_t level = 2; level < MaxSequenceLength; ++level) {
            this->generateMoreLevels(level, index_cursor);
        }
    }

    size_t ImplicitSymbols::generateLevelZero(size_t& index_cursor) {
        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (momentMatrix.UniqueSequences.size() < 2) {
            throw errors::bad_implicit_symbol({0, 0, 0}, "Zero and One should be defined in MomentMatrix.");
        }
        const auto& oneSeq = momentMatrix.UniqueSequences[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::bad_implicit_symbol({0, 0, 0}, "Identity symbol was improperly defined in MomentMatrix.");
        }

        // Construct level 0 with just normalization
        this->probabilityTable->tableData.emplace_back(1, SymbolCombo{{1, 1.0}});
        this->probabilityTable->indices.set({0, 1});
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
                    throw errors::bad_implicit_symbol({mmt.Index(), 0},
                                                      std::string("Correlation table can only be generated when")
                                                                  + " all measurements are complete.");
                }

                // PRECONDITION: Measurement has one more outcome than defined operators...
                if (mmt.num_outcomes != (mmt.num_operators()+1)) {
                    [[unlikely]]
                    throw errors::bad_implicit_symbol({mmt.Index(), 0},
                        "Measurement should have one more outcome than explicit operators.");
                }

                // Get explicit outcomes
                auto mmtSymb = this->cgForm.get_global({static_cast<size_t>(mmt.Index().global_mmt)});
                if (mmtSymb.size() != mmt.num_operators()) {
                    throw errors::bad_implicit_symbol(PMOIndex{mmt.Index(), 0},
                                                      "Could not find measurement in Collins-Gisin table.");
                }

                // Explicit outcomes:
                SymbolCombo::data_t finalOutcome{{1, 1.0}};
                for (uint32_t outcome = 0; outcome < mmt.num_operators(); ++outcome) {
                    const PMOIndex indices{mmt.Index(), outcome};

                    // Read symbol from Collins-Gisin object
                    const auto symbol_id = mmtSymb[outcome];
                    this->probabilityTable->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
                    finalOutcome.push_back({symbol_id, -1.0});
                    ++level_one_count;
                }

                // Add final measurement outcome, which is linear sum of remaining outcomes
                const PMOIndex finalIndex{mmt.Index(), static_cast<uint32_t>(mmt.num_outcomes-1)};
                this->probabilityTable->tableData.emplace_back(-1, LinearCombo(std::move(finalOutcome)));
                ++level_one_count;

                // Make index for measurement
                std::vector<size_t> index{static_cast<size_t>(mmt.Index().global_mmt)};
                this->probabilityTable->indices.set(index, {mmt_index_start, mmt_index_start + mmt.num_outcomes});
                index_cursor += mmt.num_outcomes;
            }
        }
        assert(index_cursor == this->probabilityTable->tableData.size());
        return level_one_count;
    }



    size_t ImplicitSymbols::generateMoreLevels(const size_t level, size_t& index_cursor) {
        if (this->MaxSequenceLength < level) {
            return 0;
        }

        const size_t init_cursor  = index_cursor;
        std::vector<PMODefinition> entries;
        RecursiveDoubleIndex indices{context.measurement_count(), level};

        // Iterate through party combinations:
        CombinationIndexIterator index_combo{this->context.Parties.size(), level};
        while (!index_combo.done()) {
            // Choose parties from indices
            const auto& partyIndices = *index_combo;
            assert(partyIndices.size() == level);
            MultiMmtIterator::party_list_t pmiStack;
            for (size_t i = 0; i < level; ++i) {
                pmiStack.emplace_back(&(this->context.Parties[partyIndices[i]]));
            }

            // Iterate through measurements of chosen parties
            MultiMmtIterator partyStack(this->context, std::move(pmiStack));
            while (!partyStack.done()) {
                this->generateFromCurrentStack(partyStack, index_cursor, entries, indices);
                ++partyStack;
            }

            // Next combination of party indices
            ++index_combo;
        }

        assert(index_cursor >= init_cursor);
        return index_cursor - init_cursor;
    }


    size_t ImplicitSymbols::generateFromCurrentStack(const MultiMmtIterator& stack,
                                                     size_t& index_cursor,
                                                     std::vector<PMODefinition>& entries,
                                                     RecursiveDoubleIndex& indices) {
        const size_t level = stack.dimension();
        const size_t num_outcomes = stack.count_outcomes();

        auto outcomeIter = stack.begin_outcomes();
        const auto outcomeIterEnd = stack.end_outcomes();

        // Get bulk of data from CG matrix
        auto implicit_full_opers = this->cgForm.get_global(stack.global_indices());
        assert(implicit_full_opers.size() == stack.count_operators());

        while (outcomeIter != outcomeIterEnd) {

            const size_t num_implicit = outcomeIter.implicit_count();
            if (num_implicit == 0) {
                assert(outcomeIter.explicit_op_index() < implicit_full_opers.size());
                const auto symbol_id = implicit_full_opers[outcomeIter.explicit_op_index()];

                entries.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
            } else {
                auto implMmtOut = outcomeIter.implicit_indices();
                auto explMmtOut = outcomeIter.explicit_indices();
                assert((implMmtOut.size() + explMmtOut.size()) == level);

                SymbolCombo::data_t symbolComboData;
                symbolComboData.reserve(stack.count_operators());
                // TODO:
                double first_sign = (num_implicit % 2 == 0) ? +1. : -1.;
                for (size_t i = 0, iMax = stack.count_operators(); i < iMax; ++i) {
                    symbolComboData.emplace_back(implicit_full_opers[i], first_sign);
                }


                for (size_t missing_index = 1; missing_index < num_implicit; ++missing_index) {

                }

                entries.emplace_back(0, SymbolCombo{std::move(symbolComboData)});
            }
            ++outcomeIter;
        }

        // Add index for this mmt
        indices.set(stack.global_indices(), {index_cursor, index_cursor + num_outcomes});
        index_cursor += num_outcomes;
        assert(index_cursor == entries.size());
        return num_outcomes;
    }


}