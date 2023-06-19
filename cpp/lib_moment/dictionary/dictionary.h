/**
 * dictionary.h
 *
 * Cached lists of generated operator sequences.
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_sequence_generator.h"

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
    class Dictionary {
    private:
        mutable std::shared_mutex mutex;

        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> osgs;
        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> conj_osgs;

    public:
        const Context& context;

    public:
        explicit Dictionary(const Context& context);

        /**
         * Gets dictionary of supplied word length. Creates dictionary if it doesn't already exist.
         * Nominally thread-safe, will lock for write if new dictionary requested.
         * @param word_length The maximum number of operators in a word.
         * @return Operator sequence generator.
         */
        const OperatorSequenceGenerator& operator[](size_t word_length) const;

        /**
         * Gets largest dictionary known.
         * Nominally thread-safe.
         * @return Operator sequence generator.
         */
        [[nodiscard]] const OperatorSequenceGenerator& largest() const;

        /**
         * Gets dictionary of supplied word length in conjugated order. Creates dictionary if it doesn't already exist.
         * Nominally thread-safe, will lock for write if new dictionary requested.
         * @param word_length The maximum number of operators in a word.
         * @return Operator sequence generator.
         */
        const OperatorSequenceGenerator& conjugated(size_t word_length) const;


    };
}