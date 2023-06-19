/**
 * dictionary_map.h
 *
 * Map from index as output from an OSG, to index in a SymbolTable.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include <atomic>
#include <shared_mutex>
#include <utility>
#include <vector>


namespace Moment {

    class Context;
    class SymbolTable;

    /**
     * Map from OSG output index to symbols in table.
     */
    class DictionaryMap {
    private:
        const Context& context;
        const SymbolTable& symbols;
        mutable std::shared_mutex mutex;
        std::atomic<size_t> symbol_map_max_length;
        std::vector<symbol_name_t> symbol_map;

    public:
        explicit DictionaryMap(const Context& context, const SymbolTable& symbols);

        /**
         * Update map from OSG outputs to symbol ids
         * @param promised_new_max The length of sequence up to which are  promised to exist in the symbol table.
         * @return True if new entries added.
         */
        bool update(size_t promised_new_max);

        /**
         * Get maximum index currently generated
         */
        [[nodiscard]] inline size_t max_length() const noexcept {
            return this->symbol_map_max_length.load(std::memory_order_acquire);
        }

        /**
         * Convert the index within an operator sequence generator to an entry in a symbol table.
         * @param index The index an operator sequence generator
         * @return Pair, first: symbol id, second: true if symbol is conjugated.
         */
        std::pair<symbol_name_t, bool> operator()(size_t index) const;
    };

}