/**
 * inflation_implicit_symbols.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_implicit_symbols.h"

#include "canonical_observables.h"
#include "inflation_explicit_symbols.h"
#include "inflation_matrix_system.h"

#include "operators/common/outcome_index_iterator.h"
#include "utilities/combinations.h"

namespace Moment {
    InflationImplicitSymbols::InflationImplicitSymbols(const InflationMatrixSystem& ms)
        : ImplicitSymbols{ms.Symbols(), ms.ExplicitSymbolTable(), ms.MaxRealSequenceLength()},
          context{ms.InflationContext()}, canonicalObservables{ms.CanonicalObservables()},
          iesi{ms.ExplicitSymbolTable()}  {

        // Iterate through measurements
        size_t next_index = 0;
        for (const auto& observable : canonicalObservables) {
            const size_t data_written = this->generateFromCanonicalObservable(observable);

            // Write index
            this->indices.emplace_back(next_index);
            next_index += observable.outcomes;
        }
    }

    std::span<const PMODefinition> InflationImplicitSymbols::get(const std::span<const size_t> mmtIndices) const {
        if (mmtIndices.size() > this->MaxSequenceLength) {
            throw errors::bad_implicit_symbol("Cannot look up sequences longer than the max sequence length.");
        }
        const auto& entry = this->canonicalObservables.canonical(mmtIndices);
        const ptrdiff_t first = this->indices[entry.index];
        assert(first + entry.outcomes <= this->tableData.size());
        return {this->tableData.begin() + first, entry.outcomes};
    }

    std::span<const PMODefinition> InflationImplicitSymbols::get(const std::span<const OVIndex> mmtIndices) const {
        if (mmtIndices.size() > this->MaxSequenceLength) {
            throw errors::bad_implicit_symbol("Cannot look up sequences longer than the max sequence length.");
        }
        const auto& entry = this->canonicalObservables.canonical(mmtIndices);
        const ptrdiff_t first = this->indices[entry.index];
        assert(first + entry.outcomes <= this->tableData.size());
        return {this->tableData.begin() + first, entry.outcomes};
    }

    size_t InflationImplicitSymbols::generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable) {
        // Handle identity case
        if (canonicalObservable.empty()) {
            return this->generateLevelZero(canonicalObservable);
        }
        if (canonicalObservable.size() == 1) {
            return this->generateLevelOne(canonicalObservable);
        }

        return this->generateMoreLevels(canonicalObservable);
    }


    size_t InflationImplicitSymbols::generateLevelZero(const CanonicalObservable& canonicalObservable) {
        assert(canonicalObservable.empty());
        this->tableData.emplace_back(
                1, SymbolCombo{{1, 1.0}}
        );
        return 1;
    }

    size_t InflationImplicitSymbols::generateLevelOne(const CanonicalObservable& canonicalObservable) {
        assert(canonicalObservable.size() == 1);
        const size_t initial_index = this->tableData.size();

        const auto& mmt = this->context.Observables()[canonicalObservable.indices[0].observable];
        const auto& variant = mmt.variants[canonicalObservable.indices[0].variant];

        // Measurement with one or fewer outcomes doesn't define any symbols
        if (mmt.outcomes <= 1) {
            return 0;
        }
        const size_t expected_op_count = mmt.outcomes - 1;

        // Get explicit outcomes
        auto mmtSymb = this->iesi.get(canonicalObservable.flattened_indices);
        if (mmtSymb.size() != expected_op_count) {
            throw errors::bad_implicit_symbol("Query to explicit symbol index returned incorrect number of outcomes.");
        }

        // Explicit outcomes:
        SymbolCombo::data_t finalOutcome{{1, 1.0}};
        for (uint32_t outcome = 0; outcome < (mmt.outcomes - 1); ++outcome) {
            // Read explicit symbol
            const auto symbol_id = mmtSymb[outcome].symbol_id;
            this->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
            finalOutcome.push_back({symbol_id, -1.0});
        }

        // Add final measurement outcome, which is linear sum of remaining outcomes
        this->tableData.emplace_back(-1, LinearCombo(std::move(finalOutcome)));

        return this->tableData.size() - initial_index;
    }

    size_t InflationImplicitSymbols::generateMoreLevels(const CanonicalObservable& canonicalObservable) {
        const size_t initial_index = this->tableData.size();
        std::vector<size_t> outcomes_per_measurement
            = this->context.outcomes_per_observable(canonicalObservable.indices);

        OutcomeIndexIterator outcomeIter{outcomes_per_measurement};

        while (!outcomeIter.done()) {
            const size_t num_implicit = outcomeIter.implicit_count();

            if (num_implicit == 0) {
                // No implicit operators, just put down the explicit operator
                const auto implicit_full_opers = this->iesi.get(canonicalObservable.indices);
                assert(outcomeIter.explicit_outcome_index() < implicit_full_opers.size());

                const auto symbol_id = implicit_full_opers[outcomeIter.explicit_outcome_index()].symbol_id;
                this->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});

                // Early exit
                ++outcomeIter;
                continue;
            }

            const size_t level = canonicalObservable.indices.size();
            SymbolCombo::map_t  symbolComboData;
            //SymbolCombo::data_t symbolComboData;
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
                                lookupIndices.push_back(canonicalObservable.flattened_indices[i]);
                                outcomeIndices.push_back(-1);
                            }
                            ++mNum;
                        } else {
                            // Push explicit measurement
                            lookupIndices.push_back(canonicalObservable.flattened_indices[i]);
                            outcomeIndices.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
                        }
                    }

                    // Look up, and copy with sign
                    const auto symbolsSpan = this->esiForm.get(lookupIndices, outcomeIndices);
                    for (auto symb : symbolsSpan) {
                        auto [existingIter, inserted] = symbolComboData.insert(std::pair(symb.symbol_id, the_sign));
                        if (!inserted) {
                            existingIter->second += the_sign;
                        }
                    }
                    ++partitions;
                }
                the_sign = -the_sign;
            }

            // Finally, find the "Normalization" term
            assert(the_sign == 1); // If correctly alternating, normalization should be positive always.
            std::vector<size_t> normIndices;
            std::vector<symbol_name_t> normOutcomes;
            for (size_t i = 0; i < level; ++i) {
                if (!outcomeIter.implicit(i)) {
                    normIndices.push_back(canonicalObservable.flattened_indices[i]);
                    normOutcomes.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
                }
            }
            auto normMmtSpan = this->esiForm.get(normIndices, normOutcomes);
            assert(normMmtSpan.size() == 1);
            auto [existingIter, inserted] = symbolComboData.insert(std::pair(normMmtSpan[0].symbol_id, the_sign));
            if (!inserted) {
                existingIter->second += the_sign;
            }

            // Add constructed representation to data table
            this->tableData.emplace_back(-1, SymbolCombo{symbolComboData});

            ++outcomeIter;
        }
        return this->tableData.size() - initial_index;
    }

    std::span<const PMODefinition> InflationImplicitSymbols::Block(const size_t index) const noexcept {
        assert(index < this->indices.size());
        const ptrdiff_t initial = this->indices[index];
        const auto final = static_cast<ptrdiff_t>(((index+1) < this->indices.size())
                                                    ? this->indices[index+1] : this->tableData.size());
        const size_t block_size = final - initial;

        return {this->tableData.begin() + initial, block_size};
    }
}