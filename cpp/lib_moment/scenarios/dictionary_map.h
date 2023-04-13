/**
 * dictionary_map.h
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
         * @param table The symbol table.
         * @return True if new entries added.
         */
        bool update();

        /**
         * Convert the index within an operator sequence generator to an entry in a symbol table.
         * @param index The index an operator sequence generator
         * @return Pair, first: symbol id, second: true if symbol is conjugated.
         */
        std::pair<symbol_name_t, bool> operator()(size_t index) const;
    };

}