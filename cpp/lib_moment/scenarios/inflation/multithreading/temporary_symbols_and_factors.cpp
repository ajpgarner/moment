/**
 * temporary_symbols_and_factors.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "temporary_symbols_and_factors.h"

#include "symbolic/symbol_table.h"
#include "../factor_table.h"


namespace Moment::Multithreading {
    TemporarySymbolsAndFactors::TemporarySymbolsAndFactors(SymbolTable& symbols, Inflation::FactorTable& factors)
        : symbols{symbols}, factors{factors},
        first_symbol_id{static_cast<symbol_name_t>(symbols.size())}, next_symbol_id{first_symbol_id} {

    }

    const std::vector<symbol_name_t>& TemporarySymbolsAndFactors::find_factors_by_symbol_id(const symbol_name_t symbol_id) {

        // We never do look-up of new factors
        // Since these symbols already exist, there is no contention.
        assert (symbol_id < this->first_symbol_id);

        return this->factors[symbol_id].canonical.symbols;
    }

    symbol_name_t TemporarySymbolsAndFactors::find_or_register_factors(std::span<const symbol_name_t> joint_factors) {
        // First, try uncontentious look-up in existing factor index tree
        if (auto maybe_val = this->factors.find_index_by_factors(joint_factors); maybe_val.has_value()) {
            return maybe_val.value();
        }

        // If fails, we have to look in new factors with mutex
        auto read_lock = this->get_read_lock();

        // Can we directly find node?
        auto [hint_tree, hint_factors] = this->index_tree.find_node_or_return_hint(joint_factors);
        if (hint_factors.empty()) {
            assert (hint_tree->value().has_value());
            return hint_tree->value().value();
        }

        // Did not find, so we have to upgrade our lock
        read_lock.unlock();

        // Do memory assignment first (if we are scooped, we throw it away - but we want to minimize holding lock).
        auto new_factor_str = std::make_unique<std::vector<symbol_name_t>>(joint_factors.begin(), joint_factors.end());

        // Now, upgrade lock
        auto write_lock = this->get_write_lock();

        // Check again on subtree with hint [make sure racing thread hasn't already created!]
        if (auto maybe_symbol_index = hint_tree->find(hint_factors); maybe_symbol_index.has_value()) {
            return maybe_symbol_index.value();
        }

        // We have to create a new symbol, and list it as known in our tree.
        const auto registered_id = this->next_symbol_id;
        this->index_tree.add(joint_factors, registered_id);
        this->new_factors.push_back(std::move(new_factor_str));

        // Get ID.
        ++this->next_symbol_id;
        return registered_id;
    }

    void TemporarySymbolsAndFactors::register_new_symbols_and_factors() {
        // Nothing new created, do nothing.
        const auto new_symbol_count = this->next_symbol_id - this->first_symbol_id;
        if (new_symbol_count == 0) {
            return;
        }
        assert(new_symbol_count > 0);
        assert(this->symbols.size() == this->first_symbol_id);

        // Register new symbols in table
        this->symbols.create(new_symbol_count, true, false);

        // TODO: 'merge in'

        // Register factors of symbols
        for (symbol_name_t index = 0; index < new_symbol_count; ++index) {
            auto& factor_ptr = this->new_factors[index];
            assert(factor_ptr);
            symbol_name_t new_symbol_id = this->first_symbol_id + index;
            this->factors.register_new(new_symbol_id, std::move(*factor_ptr));

            if constexpr (debug_mode) {
                factor_ptr.reset();
            }
        }

    }


}