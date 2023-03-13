/**
 * wordlist.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/operator_sequence_generator.h"

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace Moment {
    class Context;

    /**
     * Cached operator sequence generators.
     */
    class WordList {
    private:
        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> osgs;
        mutable std::vector<std::unique_ptr<OperatorSequenceGenerator>> conj_osgs;
        mutable std::shared_mutex mutex;

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

    };
}