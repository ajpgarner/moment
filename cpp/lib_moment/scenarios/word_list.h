/**
 * wordlist.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/operator_sequence_generator.h"

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace Moment {
    class Context;
    class SymbolTable;

    /**
     * Cached operator sequence generators.
     *
     * Design assumption: if k < k', then osg(k) is a prefix of osg(k').
     */
    class WordList {
    private:
        mutable std::shared_mutex mutex;

        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> osgs;
        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> conj_osgs;

        std::atomic<size_t> symbol_map_max_length;
        std::vector<symbol_name_t> symbol_map;


    public:
        const Context& context;

    public:
        WordList(const Context& context);

        /**
         * Gets dictionary of supplied word length. Creates dictionary if it doesn't already exist.
         * Nomimally thread-safe, will lock for write if new dictionary requested.
         * @param word_length The maximum number of operators in a word.
         * @return Operator sequence generator.
         */
        const OperatorSequenceGenerator& operator[](size_t word_length) const;

        /**
         * Gets dictionary of supplied word length in conjugated order. Creates dictionary if it doesn't already exist.
         * Nomimally thread-safe, will lock for write if new dictionary requested.
         * @param word_length The maximum number of operators in a word.
         * @return Operator sequence generator.
         */
        const OperatorSequenceGenerator& conjugated(size_t word_length) const;

        /**
         * Stores map of registered symbols
         */
        bool update_symbol_map(const SymbolTable& table);

        /**
         * Convert the index within an operator sequence generator to an entry in a symbol table.
         * @param index The index an operator sequence generator
         * @return Pair, first: symbol id, second: true if symbol is conjugated.
         */
        std::pair<symbol_name_t, bool> osg_index_to_symbol(size_t index) const;

    };
}