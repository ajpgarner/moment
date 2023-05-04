/**
 * representation_mapper.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "representation_mapper.h"

#include "integer_types.h"

#include "matrix/operator_sequence_generator.h"

#include "scenarios/context.h"
#include "scenarios/operator_sequence.h"

#include "utilities/ipow.h"
#include "utilities/kronecker_power.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <unsupported/Eigen/KroneckerProduct>

#include <stdexcept>

namespace Moment::Symmetrized {

    namespace {

        [[nodiscard]] inline size_t get_osg_length(const Context& context, size_t word_length) {
            const auto& osg = context.operator_sequence_generator(word_length);
            return osg.size();
        }

        std::vector<size_t> vector_remap(const Context& context,
                                         const size_t lhs_word_length,
                                         const size_t rhs_word_length,
                                         const size_t target_word_length) {
            assert(lhs_word_length + rhs_word_length == target_word_length);
            assert(lhs_word_length >= rhs_word_length);

            // First, reverse OSG to get map from hash to order.
            const auto& osg = context.operator_sequence_generator(target_word_length);
            std::map<size_t, size_t> hash_to_index;
            size_t osg_index = 0;
            for (const auto& seq : osg) {
                hash_to_index.emplace_hint(
                        hash_to_index.end(),
                        std::make_pair(seq.hash(), osg_index)
                );
                ++osg_index;
            }

            std::vector<size_t> output;

            // Now, get parents OSGs.
            const auto& lhs_osg = context.operator_sequence_generator(lhs_word_length);
            const auto& rhs_osg = context.operator_sequence_generator(rhs_word_length);

            for (const auto& lhs : lhs_osg) {
                for (const auto& rhs : rhs_osg) {
                    const auto combined = lhs * rhs;
                    const auto remap_iter = hash_to_index.find(combined.hash());
                    assert(remap_iter != hash_to_index.cend());
                    output.emplace_back(remap_iter->second);
                }
            }

            return output;
        }


        repmat_t make_lhs(const std::vector<size_t>& remap, size_t raw_dim, size_t remapped_dim) {
            assert(raw_dim == remap.size());
            repmat_t output(static_cast<int>(remapped_dim), static_cast<int>(raw_dim)); // rows, cols
            std::vector<Eigen::Triplet<double>> triplets;

            size_t true_index = 0;
            for (const auto mapped_index : remap) {
                triplets.emplace_back(static_cast<int>(mapped_index),   // row
                                      static_cast<int>(true_index), // col
                                      1.0);
                ++true_index;
            }

            output.setFromTriplets(triplets.begin(), triplets.end());

            return output;
        }

        repmat_t make_rhs(const std::vector<size_t>& remap, size_t raw_dim, size_t remapped_dim) {
            assert(raw_dim == remap.size());
            repmat_t output(static_cast<int>(raw_dim), static_cast<int>(remapped_dim)); // rows, cols
            std::vector<Eigen::Triplet<double>> triplets;

            size_t true_index = 0;
            size_t expected_index = 0;
            for (const auto mapped_index : remap) {
                const bool remapped = mapped_index != expected_index;

                if (!remapped) {
                    triplets.emplace_back(static_cast<int>(true_index),   // row
                                          static_cast<int>(expected_index), // col
                                          1.0);
                    ++expected_index;
                }

                ++true_index;
            }

            output.setFromTriplets(triplets.begin(), triplets.end());
            return output;
        }

    }

    RepresentationMapper::RepresentationMapper(const Context &context)
        : context{context},  target_word_length{1} {

        // Level one is already condensed!
        this->remapped_dim = get_osg_length(context, target_word_length);
        this->left_input_dim = this->remapped_dim;
        this->right_input_dim = 1;
        this->raw_dim = this->remapped_dim;

        // Remap is identity
        this->remap.resize(this->raw_dim);
        std::iota(this->remap.begin(), this->remap.end(), 0);
    }

    RepresentationMapper::RepresentationMapper(const Context &context,
                                               const RepresentationMapper& parent_A,
                                               const RepresentationMapper& parent_B,
                                               const size_t max_word_length)
        : context{context}, target_word_length{max_word_length} {
        assert(max_word_length > 1);
        assert(parent_A.target_word_length + parent_B.target_word_length == max_word_length);


        // Raw dimension comes from product of parent outputs
        this->left_input_dim = parent_A.remapped_dim;
        this->right_input_dim = parent_B.remapped_dim;
        this->raw_dim = this->left_input_dim * this->right_input_dim;

        // Remapped dimension comes from OSG at level.
        const auto& osg = context.operator_sequence_generator(target_word_length);
        this->remapped_dim = osg.size();

        // Make vector remap
        this->remap = vector_remap(context,
                                   parent_A.target_word_length,
                                   parent_B.target_word_length,
                                   this->target_word_length);

        this->lhs = make_lhs(this->remap, this->raw_dim, this->remapped_dim);
        this->rhs = make_rhs(this->remap, this->raw_dim, this->remapped_dim);
    }

    repmat_t RepresentationMapper::operator()(const repmat_t &matrix) const {
        assert(this->target_word_length > 1);
        assert(this->left_input_dim == this->right_input_dim);
        return this->operator()(matrix, matrix);
    }


    repmat_t RepresentationMapper::operator()(const repmat_t& elem_r1, const repmat_t& elem_r2) const {
        assert(elem_r1.cols() == this->left_input_dim);
        assert(elem_r1.rows() == this->left_input_dim);
        assert(elem_r2.cols() == this->right_input_dim);
        assert(elem_r2.rows() == this->right_input_dim);

        return this->lhs * Eigen::kroneckerProduct(elem_r1, elem_r2) * this->rhs;
    }



}