/**
 * inflation_explicit_symbols.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_explicit_symbols.h"

#include "canonical_observables.h"
#include "inflation_context.h"
#include "inflation_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "utilities/combinations.h"
#include "utilities/multi_dimensional_index_iterator.h"


namespace Moment {

    namespace {
        std::vector<size_t> makeOpCounts(const InflationContext& context) {
            std::vector<size_t> output;
            output.reserve(context.observable_variant_count());
            for (const auto& o : context.Observables()) {
                std::fill_n(std::back_inserter(output), o.variant_count, o.operators());
            }
            assert(output.size() == context.observable_variant_count());
            return output;
        }
    }

    InflationExplicitSymbolIndex::InflationExplicitSymbolIndex(const InflationMatrixSystem &matrixSystem,
                                                               const size_t level)
            : ExplicitSymbolIndex{level, makeOpCounts(matrixSystem.InflationContext())},
              canonicalObservables{matrixSystem.CanonicalObservables()}  {

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

        // Manually add ID
        this->data.emplace_back(ExplicitSymbolEntry{oneSeq.Id(), oneSeq.basis_key().first});
        this->indices.emplace_back(0);

        // For each measurement in canonical symbols
        for (const auto& canonObs : this->canonicalObservables) {
            // Skip ID
            if (canonObs.indices.empty()) {
                // (should only be one ID)
                assert(this->data.size() == 1);
                continue;
            }

            // Count operators associated with chosen parties
            std::vector<size_t> opers_per_observable;
            opers_per_observable.reserve(canonObs.indices.size());
            for (const auto ovIndex : canonObs.indices) {
                opers_per_observable.emplace_back(context.Observables()[ovIndex.observable].operators());
            }
            const auto num_operators = canonObs.operators;

            // Get start of data block
            const auto data_start_index = static_cast<ptrdiff_t>(this->data.size());
            this->data.reserve(this->data.size() + num_operators);

            // Now, iterate over every operator sequence within measurement
            MultiDimensionalIndexIterator opIndicesIter{opers_per_observable, false};
            const MultiDimensionalIndexIterator opIndicesIterEnd{opers_per_observable, true};
            while (opIndicesIter != opIndicesIterEnd) {
                // Find operator sequence
                auto opIndices = *opIndicesIter;
                std::vector<oper_name_t> op_str;
                op_str.reserve(opIndices.size());
                for (size_t i = 0; i < opIndices.size(); ++i) {
                    const auto& obs = observables[canonObs.indices[i].observable];
                    const auto& var = obs.variants[canonObs.indices[i].variant];
                    op_str.emplace_back(var.operator_offset + opIndices[i]);
                }
                OperatorSequence opSeq{std::move(op_str), context};

                // Find associated symbol with operator sequence
                auto symbol_loc = symbols.where(opSeq);
                if (symbol_loc == nullptr) {
                    throw errors::cg_form_error{"Could not find expected symbol in MomentMatrix."};
                }
                this->data.emplace_back(ExplicitSymbolEntry{symbol_loc->Id(), symbol_loc->basis_key().first});

                ++opIndicesIter;
            }

            // Add index
            this->indices.push_back(data_start_index);
        }
    }

    std::span<const ExplicitSymbolEntry> InflationExplicitSymbolIndex::get(std::span<const size_t> mmtIndices) const {
        const auto& entry = this->canonicalObservables.canonical(mmtIndices);
        const auto first = this->indices[entry.index];
        assert(first + entry.operators <= this->data.size());
        return {this->data.begin() + first, entry.operators};
    }

    std::span<const ExplicitSymbolEntry> InflationExplicitSymbolIndex::get(std::span<const OVIndex> mmts) const {
        const auto& entry = this->canonicalObservables.canonical(mmts);
        const auto first = this->indices[entry.index];
        assert(first + entry.operators <= this->data.size());
        return {this->data.begin() + first, entry.operators};
    }
}