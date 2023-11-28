/**
 * extension_suggester.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "extension_suggester.h"

#include "factor_table.h"
#include "inflation_context.h"

#include "matrix/monomial_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "dictionary/operator_sequence_generator.h"
#include "symbolic/symbol_table.h"

namespace Moment::Inflation {
    ExtensionSuggester::ExtensionSuggester(const InflationContext& context,
                                           const SymbolTable& symbols,
                                           const FactorTable& factors)
        : context{context}, symbols{symbols}, factors{factors} { }

    std::set<symbol_name_t> ExtensionSuggester::operator()(const SymbolicMatrix &matrix) const {
        if (const auto* mmPtr = dynamic_cast<const MonomialMatrix*>(&matrix); mmPtr != nullptr) {
            return (*this)(*mmPtr);
        }

        throw std::invalid_argument{"Can only suggest extensions for monomial moment matrices."};
    }

    std::set<symbol_name_t> ExtensionSuggester::operator()(const MonomialMatrix& matrix) const {
        assert(&matrix.symbols == &this->symbols);

        MomentMatrix const * mm_ptr = MomentMatrix::to_operator_matrix_ptr(matrix);
        if (nullptr == mm_ptr) {
            throw std::invalid_argument{"Can only suggest extensions for monomial moment matrices."};
        }
        const auto& moment_matrix = *mm_ptr;


        DynamicBitset<uint64_t> tested_factors{this->symbols.size()};
        DynamicBitset<uint64_t> chosen_factors{this->symbols.size()};

        auto necessary_factors = nonfundamental_symbols(matrix);

        size_t extension_count = 0;


        // Return if nothing needs factorizing
        if (necessary_factors.empty()) {
            return {};
        }

        while ((extension_count < max_extensions) && !(necessary_factors.empty())) {
            // 1. choose factor of some non-fundamental string
            const auto trial_factor_symbol = get_symbol_to_test(necessary_factors, tested_factors);
            if (trial_factor_symbol < 0) {
                break;
            }

            // 2. see what constraints introducing this extension could impose
            bool any_use = false;
            for (const auto &rawPrefix: moment_matrix.generators()()) {
                // Find prefix as factored object
                auto prefix = context.canonical_moment(rawPrefix);

                auto [source_sym_index, source_conj] = symbols.hash_to_index(prefix.hash());
                assert(source_sym_index != std::numeric_limits<ptrdiff_t>::max());
                const auto &source_factors = factors[source_sym_index].canonical.symbols;

                // See if multiplying prefix by chosen factor yields a known symbol
                auto joint_factors = FactorTable::combine_symbolic_factors(source_factors, {trial_factor_symbol});
                auto maybe_symbol_index = factors.find_index_by_factors(joint_factors);
                if (!maybe_symbol_index.has_value()) {
                    continue;
                }
                // Do we need this one?
                if (necessary_factors.test(maybe_symbol_index.value())) {
                    // If it does, then we check the symbol off as generated, and register suggested symbol as useful
                    necessary_factors.unset(maybe_symbol_index.value());
                    any_use = true;
                }
            }

            tested_factors.set(trial_factor_symbol);
            if (any_use) {
                chosen_factors.set(trial_factor_symbol);
            }

            ++extension_count;
        }

        return chosen_factors.to_set<symbol_name_t>();
    }

    DynamicBitset<uint64_t> ExtensionSuggester::nonfundamental_symbols(const MonomialMatrix &matrix) const {
        DynamicBitset<uint64_t> expressions(this->symbols.size());
        for (auto symbol_id : matrix.IncludedSymbols()) { 
            const auto& factor_info = factors[symbol_id];
            if (!factor_info.fundamental()) {
                expressions.set(symbol_id);
            }
        }
        return expressions;
    }

    symbol_name_t ExtensionSuggester::get_symbol_to_test(const DynamicBitset<uint64_t> &necessary_factors,
                                                         const DynamicBitset<uint64_t> &tested_factors) const {

        // Find new symbol
        for (auto nf_index : necessary_factors) {
            const auto &nonfundamental_object = this->factors[nf_index];
            for (auto possible_factor : nonfundamental_object.canonical.symbols) {
                if (!tested_factors.test(possible_factor)) {
                    // Factor should be fundamental, and hence not in list
                    assert(!necessary_factors.test(possible_factor));
                    return possible_factor;
                }
            }
        }

        // No further suggestions
        return -1;
    }

}