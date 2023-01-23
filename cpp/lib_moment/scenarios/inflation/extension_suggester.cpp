/**
 * extension_suggester.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "extension_suggester.h"

#include "factor_table.h"

#include "matrix/symbolic_matrix.h"
#include "matrix/moment_matrix.h"
#include "matrix/operator_sequence_generator.h"
#include "symbolic/symbol_table.h"


namespace Moment::Inflation {
    ExtensionSuggester::ExtensionSuggester(const SymbolTable& symbols, const FactorTable& factors)
        : symbols{symbols}, factors{factors} { }

    std::set<symbol_name_t> ExtensionSuggester::operator()(const MomentMatrix& matrix) const {
        assert(&matrix.Symbols == &this->symbols);
        std::set<symbol_name_t> output;

        auto necessary_factors = nonfundamental_symbols(matrix);

        size_t extension_count = 0;

        // Return if nothing needs factorizing
        if (necessary_factors.empty()) {
            return output;
        }

        while ((extension_count < max_extensions) && !(necessary_factors.empty())) {

            // 1. choose factor of some non-fundamental string
            const auto &nonfundamental_object = this->factors[*necessary_factors.begin()];
            assert(!nonfundamental_object.canonical.symbols.empty());
            auto trial_factor_symbol = nonfundamental_object.canonical.symbols.front();
            assert(!necessary_factors.test(trial_factor_symbol)); // Factor should be fundamental, and hence not in list

            // 2. see what constraints introducing this extension could impose
            for (const auto &prefix: matrix.Generators()) {
                // Find prefix as factored object
                auto [source_sym_index, source_conj] = symbols.hash_to_index(prefix.hash());
                const auto &source_factors = factors[source_sym_index].canonical.symbols;

                // See if multiplying prefix by chosen factor yields a known symbol
                auto joint_factors = FactorTable::combine_symbolic_factors(source_factors, {trial_factor_symbol});
                auto maybe_symbol_index = factors.find_index_by_factors(joint_factors);
                if (!maybe_symbol_index.has_value()) {
                    continue;
                }

                // If it does, then we check the symbol off as generated, and register suggested symbol as useful
                necessary_factors.unset(maybe_symbol_index.value());
                output.insert(trial_factor_symbol);
            }

            ++extension_count;
        }

        return output;
    }

    DynamicBitset<uint64_t> ExtensionSuggester::nonfundamental_symbols(const SymbolicMatrix &matrix) const {
        DynamicBitset<uint64_t> expressions(this->symbols.size());
        for (auto symbol_id : matrix.SMP().RealSymbols()) {
            const auto& factor_info = factors[symbol_id];
            if (!factor_info.fundamental()) {
                expressions.set(symbol_id);
            }
        }
        return expressions;
    }

}