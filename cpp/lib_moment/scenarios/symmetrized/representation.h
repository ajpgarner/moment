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

namespace Moment::Symmetrized {

    using repmat_t = Eigen::SparseMatrix<double>;

    class Representation {
    public:
        /** The size of each matrix */
        const size_t dimension;

        /** The longest word represented by this representation */
        const size_t word_length;

    private:
        /** The group elements. */
        std::vector<repmat_t> elements;

        /** Sum of all group elements. */
        repmat_t sum_of_elements;

    public:
        explicit Representation(size_t word_length, std::vector<repmat_t>&& entries);

        Representation(Representation&& rhs) = default;

        [[nodiscard]] const repmat_t& operator[](size_t idx) const {
            assert(idx < this->elements.size());
            return this->elements[idx];
        }

        [[nodiscard]] inline const auto& sum_of() const { return this->sum_of_elements; }

        [[nodiscard]] inline const auto& group_elements() const noexcept { return this->elements; }

        [[nodiscard]] inline bool empty() const noexcept { return this->elements.empty(); }

        [[nodiscard]] inline auto size() const noexcept { return this->elements.size(); }

        [[nodiscard]] inline auto begin() const noexcept { return this->elements.cbegin(); }

        [[nodiscard]] inline auto end() const noexcept { return this->elements.cend(); }




    };
}