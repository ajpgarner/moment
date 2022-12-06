/**
 * inflation_implicit_symbols.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_implicit_symbols.h"

#include "canonical_observables.h"
#include "inflation_explicit_symbols.h"
#include "inflation_matrix_system.h"

namespace NPATK {
    InflationImplicitSymbols::InflationImplicitSymbols(const InflationMatrixSystem& ms)
        : ImplicitSymbols{ms.Symbols(), ms.ExplicitSymbolTable(), ms.MaxRealSequenceLength()},
          context{ms.InflationContext()}  {

        // Iterate through measurements
        const auto& co = ms.CanonicalObservables();
        size_t next_index = 0;
        for (const auto& observable : co) {
            // Write explicit data
            this->generateFromCanonicalObservable(observable);

            // Write index
            this->indices.emplace_back(next_index);
            next_index += observable.outcomes;
        }

        //ms.ExplicitSymbolTable();

    }



    size_t InflationImplicitSymbols::generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable) {
//
//        const size_t level = stack.count_indices();
//        const size_t num_outcomes = stack.count_outcomes();
//
//
//
//        for (auto outcomeIter = stack.begin_outcomes(), outcomeIterEnd = stack.end_outcomes();
//             outcomeIter != outcomeIterEnd; ++outcomeIter) {
//            const size_t num_implicit = outcomeIter.implicit_count();
//            if (num_implicit == 0) {
//                // No implicit operators, just put down the explicit operator
//                const auto implicit_full_opers = this->esiForm.get(stack.global_indices());
//                assert(implicit_full_opers.size() == stack.count_operators());
//                assert(outcomeIter.explicit_outcome_index() < implicit_full_opers.size());
//                const auto symbol_id = implicit_full_opers[outcomeIter.explicit_outcome_index()].symbol_id;
//                this->tableData.emplace_back(symbol_id, SymbolCombo{{symbol_id, 1.0}});
//            } else {
//                SymbolCombo::data_t symbolComboData;
//                double the_sign = (num_implicit % 2 == 0) ? +1. : -1.;
//                for (size_t missing_index = num_implicit; missing_index > 0; --missing_index) {
//                    PartitionIterator partitions{num_implicit, missing_index};
//                    while (!partitions.done()) {
//
//                        // Build measurement index query:
//                        std::vector<size_t> lookupIndices;
//                        std::vector<symbol_name_t> outcomeIndices;
//                        size_t mNum = 0;
//                        for (size_t i = 0; i < level; ++i) {
//                            if (outcomeIter.implicit(i)) {
//                                // Implicit measurement: either push -1, or skip
//                                if (partitions.bits(mNum)) {
//                                    lookupIndices.push_back(stack.global_indices()[i]);
//                                    outcomeIndices.push_back(-1);
//                                }
//                                ++mNum;
//                            } else {
//                                // Push explicit measurement
//                                lookupIndices.push_back(stack.global_indices()[i]);
//                                outcomeIndices.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
//                            }
//                        }
//
//                        // Look up, and copy with sign
//                        const auto symbolsSpan = this->esiForm.get(lookupIndices, outcomeIndices);
//                        for (auto symb : symbolsSpan) {
//                            symbolComboData.emplace_back(symb.symbol_id, the_sign);
//                        }
//                        ++partitions;
//                    }
//                    the_sign = -the_sign;
//                }
//
//                // Finally, find the "Normalization" term
//                assert(the_sign == 1); // If correctly alternating, normalization should be positive always.
//                std::vector<size_t> normIndices;
//                std::vector<symbol_name_t> normOutcomes;
//                for (size_t i = 0; i < stack.count_indices(); ++i) {
//                    if (!outcomeIter.implicit(i)) {
//                        normIndices.push_back(stack.global_indices()[i]);
//                        normOutcomes.push_back(static_cast<symbol_name_t>(outcomeIter[i]));
//                    }
//                }
//                auto normMmtSpan = this->esiForm.get(normIndices, normOutcomes);
//                assert(normMmtSpan.size() == 1);
//                symbolComboData.emplace_back(normMmtSpan[0].symbol_id, the_sign);
//
//                // Add constructed representation to data table
//                this->tableData.emplace_back(-1, SymbolCombo{std::move(symbolComboData)});
//            }
//        }
//
//        // Add index for this mmt
//        this->indices.set(stack.global_indices(), {index_cursor, index_cursor + num_outcomes});
//        index_cursor += num_outcomes;
//        assert(index_cursor == this->tableData.size());
//        return num_outcomes;
        throw std::runtime_error{"Not implemented"};
    }
}