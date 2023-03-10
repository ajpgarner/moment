/**
 * representation.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

#include <cassert>

#include <vector>

namespace Moment {

    using repmat_t = Eigen::SparseMatrix<double>;

    class Representation {
    public:
        const size_t dimension;

    private:
        std::vector<repmat_t> elements;

    public:
        explicit Representation(std::vector<repmat_t>&& entries);

        Representation(Representation&& rhs) = default;

        const repmat_t& operator[](size_t idx) const {
            assert(idx < this->elements.size());
            return this->elements[idx];
        }


    };
}