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
#include "osg_pair.h"

#include "multithreading/maintains_mutex.h"

#include <atomic>
#include <map>
#include <vector>

namespace Moment {
    class Context;
    class SymbolTable;

    /**
     * Cached operator sequence generators.
     *
     * Design assumption: if k < k', then osg(k) is a prefix of osg(k').
     */
    class Dictionary : protected MaintainsMutex {
    protected:
        /** List of operator sequences */
        mutable std::vector<OSGPair> osgs;

        /** Key, linking NPA hierarchy level (e.g. moment matrix level) to generator offset */
        mutable std::map<size_t, size_t> npa_level_to_offset;

    public:
        const Context& context;

    public:
        /**
         * Construct a cache of operator sequence generators
         * @param context
         */
        explicit Dictionary(const Context& context);

        /**
         * Polymorphic destructor.
         */
        virtual ~Dictionary() = default;

        /**
         * Gets a 'pure' NPA hierarchy level (e.g. Moment matrix) generator.
         * @param npa_level The maximum word length.
         */
        [[nodiscard]] const OSGPair& Level(const size_t max_word_length) const;

        /**
         * Alias to 'pure' NPA hierarchy level (e.g. Moment matrix) generator.
         * @param npa_level The maximum word length.
         */
        [[nodiscard]] inline const OSGPair& operator()(const size_t max_word_length) const {
            return this->Level(max_word_length);
        }

        /**
         * Return number of registered OSGs
         */
        [[nodiscard]] inline size_t size() const noexcept {
            return this->osgs.size();
        }

    };
}