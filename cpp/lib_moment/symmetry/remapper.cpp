/**
 * remapper.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "remapper.h"

#include "integer_types.h"

#include "matrix/operator_sequence_generator.h"

#include "scenarios/context.h"
#include "scenarios/operator_sequence.h"

#include "utilities/ipow.h"
#include "utilities/kronecker_power.h"
#include "utilities/multi_dimensional_index_iterator.h"

#include <stdexcept>

namespace Moment {

    namespace {

        std::vector<size_t> vector_remap(const Context& context,  const size_t target_word_length) {


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

            // Now, iterate through raw indices to make dense map from potential words to canonical index
            std::vector<size_t> output;
            auto op_count = context.size();
            for (const auto& raw_vec : MultiDimensionalIndexRange{std::vector<size_t>(target_word_length, 1+op_count)}) {
                sequence_storage_t seq_vec;
                for (const auto x : raw_vec) {
                    if (x > 0) {
                        seq_vec.emplace_back(x - 1);
                    }
                }
                OperatorSequence op_seq{std::move(seq_vec), context};

                auto remap_iter = hash_to_index.find(op_seq.hash());
                assert(remap_iter != hash_to_index.cend());
                output.emplace_back(remap_iter->second);
            }
            return output;
        }


        repmat_t make_lhs(const std::vector<size_t>& remap, size_t raw_dim, size_t remapped_dim) {
            assert(raw_dim == remap.size());
            repmat_t output(remapped_dim, raw_dim); // rows, cols
            std::vector<Eigen::Triplet<double>> triplets;

            size_t true_index = 0;
            size_t expected_index = 0;
            for (const auto mapped_index : remap) {
                const bool remapped = mapped_index != expected_index;

                if (!remapped) {
                    triplets.emplace_back(static_cast<int>(expected_index),   // row
                                          static_cast<int>(true_index), // col
                                          1.0);
                    ++expected_index;
                }

                ++true_index;
            }

            output.setFromTriplets(triplets.begin(), triplets.end());
            return output;
        }

        repmat_t make_rhs(const std::vector<size_t>& remap, size_t raw_dim, size_t remapped_dim) {
            assert(raw_dim == remap.size());
            repmat_t output(raw_dim, remapped_dim); // rows, cols
            std::vector<Eigen::Triplet<double>> triplets;

            size_t true_index = 0;
            for (const auto mapped_index : remap) {
                triplets.emplace_back(static_cast<int>(true_index),   // row
                                      static_cast<int>(mapped_index), // col
                                      1.0);
                ++true_index;
            }

            output.setFromTriplets(triplets.begin(), triplets.end());

            return output;
        }
    }

    Remapper::Remapper(const Context &context, const size_t max_word_length)
        : context{context}, target_word_length{max_word_length} {

        // Get size from OSG
        const auto& osg = context.operator_sequence_generator(target_word_length);
        this->remapped_dim = osg.size();

        // XXX: '1+op_count' will fail on contexts with single character rewrite equivalences (e.g. "b = a")
        // Get raw kroenecker size
        const auto op_count = context.size();
        this->raw_dim = ipow(1+op_count, target_word_length);

        // Make vector remap
        this->remap = vector_remap(context, target_word_length);

        this->lhs = make_lhs(this->remap, this->raw_dim, this->remapped_dim);
        this->rhs = make_rhs(this->remap, this->raw_dim, this->remapped_dim);

    }

    repmat_t Remapper::operator()(const repmat_t &matrix) const {

        repmat_t kroned = kronecker_power(matrix, target_word_length);

        return this->lhs * kroned * this->rhs;
    }

}