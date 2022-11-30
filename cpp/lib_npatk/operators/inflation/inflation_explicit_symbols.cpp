/**
 * inflation_explicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_explicit_symbols.h"

#include "inflation_context.h"
#include "inflation_matrix_system.h"

#include "operators/matrix/symbol_table.h"
#include "utilities/combinations.h"
#include "utilities/multi_dimensional_index_iterator.h"


namespace NPATK {

    namespace {

        std::vector<size_t> makeOpCounts(const InflationContext& context) {
            std::vector<size_t> output;
            output.reserve(context.Observables().size());
            for (const auto& o : context.Observables()) {
                output.emplace_back(o.outcomes-1);
            }
            return output;
        }
    }

    InflationExplicitSymbolIndex::InflationExplicitSymbolIndex(const InflationMatrixSystem &matrixSystem,
                                                               const size_t level)
            : ExplicitSymbolIndex{level, makeOpCounts(matrixSystem.InflationContext()),
                                  JointMeasurementIndex(
                                          std::vector<size_t>(matrixSystem.InflationContext().Observables().size(),
                                                              static_cast<size_t>(1)),
                                          std::min(level, matrixSystem.InflationContext().Observables().size()))} {

        const auto &context = matrixSystem.InflationContext();
        const auto &observables = context.Observables();
        const SymbolTable &symbols = matrixSystem.Symbols();

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (symbols.size() < 2) {
            throw errors::cg_form_error("Zero and One should be defined.");
        }
        const auto &oneSeq = symbols[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw errors::cg_form_error("Identity symbol was improperly defined.");
        }

        // Base level points to identity element symbol
        this->indices.set({0, 1});
        this->data.push_back({1, matrixSystem.Symbols().to_basis(1).first});
        size_t index_counter = 1;

        // For each level,
        for (size_t current_level = 1; current_level < (Level + 1); ++current_level) {

            // Iterate through party combinations:
            CombinationIndexIterator index_combo{observables.size(), current_level};
            while (!index_combo.done()) {
                // Choose parties from indices
                const auto &partyIndices = *index_combo;
                assert(partyIndices.size() == current_level);

                // Count operators associated with chosen parties
                std::vector<size_t> opers_per_observable;
                opers_per_observable.reserve(partyIndices.size());
                for (const auto party: partyIndices) {
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
}