/**
 * explicit_symbol.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_explicit_symbols.h"

#include "joint_measurement_iterator.h"
#include "locality_context.h"
#include "locality_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "utilities/combinations.h"

namespace Moment::Locality {
    namespace {
        std::vector<size_t> makeOpCounts(const LocalityContext &context) {
            std::vector<size_t> output;
            output.reserve(context.measurement_count());
            size_t i = 0;
            for (const auto &p: context.Parties) {
                for (const auto &m: p.Measurements) {
                    assert(m.Index().global_mmt == i);
                    output.push_back(m.num_operators());
                    ++i;
                }
            }
            assert(i == context.measurement_count());
            return output;
        }
    }


    LocalityExplicitSymbolIndex::LocalityExplicitSymbolIndex(const LocalityMatrixSystem& matrixSystem, const size_t level)
        : ExplicitSymbolIndex{level, makeOpCounts(matrixSystem.localityContext)},
            indices{matrixSystem.localityContext.measurements_per_party(),
                    std::min(level, matrixSystem.localityContext.Parties.size())} {

        const auto& context = matrixSystem.localityContext;
        const SymbolTable& symbols = matrixSystem.Symbols();

        // ASSERTIONS: Zero and One should be defined as unique sequences in elements 0 and 1 accordingly.
        if (symbols.size() < 2) {
            throw Moment::errors::bad_explicit_symbol{"Zero and One should be defined."};
        }
        const auto& oneSeq = symbols[1];
        if (!oneSeq.sequence().empty() || oneSeq.sequence().zero() || (oneSeq.Id() != 1)) {
            throw Moment::errors::bad_explicit_symbol{"Identity symbol was improperly defined."};
        }

        // Base level points to identity element symbol
        this->indices.set({0,1});
        this->data.push_back({1, matrixSystem.Symbols().Basis(1).first});
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
                            throw Moment::errors::bad_explicit_symbol{"Could not find expected symbol in MomentMatrix."};
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

    std::span<const ExplicitSymbolEntry> LocalityExplicitSymbolIndex::get(std::span<const size_t> mmtIndices) const {
        auto [first, last] = this->indices.access(mmtIndices);
        if ((first < 0) || (first >= last)) {
            return {this->data.begin(), 0};
        }
        assert(last <= this->data.size());
        return {this->data.begin() + first, static_cast<size_t>(last - first)};
    }



}