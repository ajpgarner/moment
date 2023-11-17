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
    public:
        /**
         * An operator sequence generator and its conjugate
         */
        struct OSGPair {
        private:
            std::unique_ptr<OperatorSequenceGenerator> forward_osg;
            std::unique_ptr<OperatorSequenceGenerator> conjugate_osg;

        public:
            explicit OSGPair(std::unique_ptr<OperatorSequenceGenerator> fwd,
                    std::unique_ptr<OperatorSequenceGenerator> rev = nullptr) noexcept
                : forward_osg{std::move(fwd)}, conjugate_osg{std::move(rev)} {
            }

            OSGPair(OSGPair&& rhs) = default;

            [[nodiscard]] const OperatorSequenceGenerator& operator()() const noexcept {
                return *forward_osg;
            }

            [[nodiscard]] const OperatorSequenceGenerator& conjugate() const noexcept {
                return (conjugate_osg ? *conjugate_osg : *forward_osg);
            }
        };

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
         * Gets a 'pure' NPA hierarchy level (e.g. Moment matrix) generator.
         * @param npa_level The maximum word length.
         */
        const Dictionary::OSGPair& Level(const size_t max_word_length) const;

    };
}