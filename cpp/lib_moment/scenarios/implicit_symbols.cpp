/**
 * implicit_symbols.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "implicit_symbols.h"


#include "scenarios/outcome_index_iterator.h"
#include "scenarios/implicit_outcome_iterator.h"

#include <algorithm>
#include <cassert>

namespace Moment {
    namespace {
        void implicit_to_explicit_level_0(const std::span<const size_t> outcomes_per_measurement,
                                          const std::span<const PMODefinition> implicit_symbols,
                                          const std::span<const double> input_values,
                                          std::map<symbol_name_t, double>& calculated_symbols) {

            ImplicitOutcomeIterator outcomeIter{outcomes_per_measurement, 0};
            while (!outcomeIter.done()) {
                const auto global_index = outcomeIter.global();
                assert(implicit_symbols[global_index].symbol_id >= 0);
                calculated_symbols.insert(std::make_pair(implicit_symbols[global_index].symbol_id,
                                                             input_values[global_index]));
                ++outcomeIter;
            }
        }

        void implicit_to_explicit_other_levels(const std::span<const size_t> outcomes_per_measurement,
                                               const std::span<const PMODefinition> implicit_symbols,
                                               const std::span<const double> input_values,
                                               size_t num_implicit,
                                               std::map<symbol_name_t, double>& calculated_symbols) {

            ImplicitOutcomeIterator outcomeIter{outcomes_per_measurement, num_implicit};
            while (!outcomeIter.done()) {
                const auto global_index = outcomeIter.global();
                assert(implicit_symbols[global_index].symbol_id == -1);
                // One implicit part, so should be a difference between two??

                double the_value = input_values[global_index];

                const auto& expression = implicit_symbols[global_index].expression;
                symbol_name_t the_symbol = -1;
                double final_weight = 1.0;

                for (auto [symbol, factor, conjugated] : implicit_symbols[global_index].expression) {
                    // Do we know value of this?
                    auto expr_iter = calculated_symbols.find(symbol);
                    if (expr_iter != calculated_symbols.end()) {
                        the_value -= expr_iter->second * factor;
                    } else {
                        assert(the_symbol == -1);
                        the_symbol = symbol;
                        final_weight = factor;
                    }
                }
                assert(the_symbol != -1);
                the_value /= final_weight;
                calculated_symbols.insert(std::make_pair(the_symbol, the_value));

                ++outcomeIter;
            }

        }
    }


    std::map<symbol_name_t, double>
    ImplicitSymbols::implicit_to_explicit(const std::span<const size_t> outcomes_per_measurement,
                                          const std::span<const PMODefinition> implicit_symbols,
                                          const std::span<const double> input_values) {

        const size_t mmt_count = outcomes_per_measurement.size();
        assert(implicit_symbols.size() == input_values.size());
        assert(std::all_of(outcomes_per_measurement.begin(), outcomes_per_measurement.end(), [](auto x){ return x > 0; }));

        std::map<symbol_name_t, double> calculated_symbols{};

        // Base symbols
        implicit_to_explicit_level_0(outcomes_per_measurement, implicit_symbols, input_values, calculated_symbols);
        if (mmt_count < 1) {
            return calculated_symbols;
        }

        // Start from no implicit symbols, and go up to 'fully' implicit
        for (size_t num_implicit = 1; num_implicit <= mmt_count; ++num_implicit) {
            implicit_to_explicit_other_levels(outcomes_per_measurement, implicit_symbols, input_values,
                                              num_implicit, calculated_symbols);
        }

        return calculated_symbols;

    }
}