/**
 * representation_mapper.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "representation.h"

#include <Eigen/Sparse>

#include <cassert>

#include <map>
#include <memory>
#include <vector>

namespace Moment {
    class Context;
}

namespace Moment::Symmetrized {

    /**
     * Utility for generating new representations of a symmetry group on a context.
     */
    class RepresentationMapper {
    public:
        using lhs_mat_t = Eigen::SparseMatrix<double, Eigen::RowMajor>;
        using rhs_mat_t = Eigen::SparseMatrix<double, Eigen::ColMajor>;

    private:
        size_t left_input_dim;
        size_t right_input_dim;
        size_t raw_dim;

        size_t remapped_dim;

        std::vector<size_t> remap;

        lhs_mat_t lhs;
        rhs_mat_t rhs;

    public:
        const Context& context;

        const size_t target_word_length;

        /**
         * Specialized constructor for representation level 1.
         * @param context The operator context.
         */
        explicit RepresentationMapper(const Context& context);

        /**
         * Constructor for general representation levels
         * @param context The operator context.
         * @param parent_A The larger parent mapper (will not hold reference).
         * @param parent_B The smaller parent mapper (will not hold reference).
         */
        RepresentationMapper(const Context& context,
                             const RepresentationMapper& parent_A,
                             const RepresentationMapper& parent_B,
                             size_t max_word_length);

        // RepresentationMapper(const Context& context, size_t max_word_length);

        /** Dimension of Kronecker product */
        [[nodiscard]] inline size_t raw_dimension() const noexcept { return this->raw_dim; }

        /** Input dimensions.
         * @return pair: first gives left input dimension, second gives right input dimension. */
        [[nodiscard]] inline std::pair<size_t, size_t> input_dimensions() const noexcept {
            return {this->left_input_dim, this->right_input_dim};
        }

        /** Dimension after equivalences applied */
        [[nodiscard]] inline size_t remapped_dimension() const noexcept { return this->remapped_dim; }

        /** The matrix that acts on the left of the Kronecker product (encoding redundant dims) */
        [[nodiscard]] inline auto LHS() const noexcept { return this->lhs; }

        /** The matrix that acts on the right of the Kronecker product (encoding remapped dims) */
        [[nodiscard]] inline auto RHS() const noexcept { return this->rhs; }

        /**
         * Get index offset.
         */
        [[nodiscard]] inline size_t operator[](size_t index) const noexcept {
            assert(index < this->remap.size());
            return this->remap[index];
        }

        /**
         * Map group matrix in parent representation to length N representation.
         * Specialization for power-of-two representations.
         * @param matrix
         * @return Group element in target representation.
         */
        [[nodiscard]] repmat_t operator()(const repmat_t& matrix) const;


        /**
         * Map group matrix in parent representation to length N representation
         * @param p1_rep The group element in the first parent representation.
         * @param p2_rep The group element in the second parent representation.
         * @return Group element in target representation.
         */
        [[nodiscard]] repmat_t operator()(const repmat_t& p1_rep, const repmat_t& p2_rep) const;


    };
}