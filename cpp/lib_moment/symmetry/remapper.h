/**
 * remapper.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>

#include <map>
#include <vector>

namespace Moment {
    class Context;

    /**
     * Utility for generating new representations of a symmetry group on a context.
     */
    class Remapper {

    private:
        std::vector<size_t> remap;

    public:
        const Context& context;

        Remapper(const Context& context, size_t max_word_length);

        inline size_t operator[](size_t index) const {
            assert(index < this->remap.size());
            return this->remap[index];
        }
    };
}