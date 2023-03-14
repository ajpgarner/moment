/**
 * remapper.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "representation.h"

#include <Eigen/Sparse>

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

        size_t raw_dim;
        size_t remapped_dim;

        repmat_t lhs;
        repmat_t rhs;

    public:
        const Context& context;
        const size_t target_word_length;

        Remapper(const Context& context, size_t max_word_length);

        /**
         * Get index offset.
         */
        inline size_t operator[](size_t index) const {
            assert(index < this->remap.size());
            return this->remap[index];
        }

        /**
         * Map group matrix in length 1 representation to length N representation
         * @param matrix
         * @return
         */
        repmat_t operator()(const repmat_t& matrix) const;
    };
}