/**
 * operator_matrix_transformation.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading/multithreading.h"
#include "multithreading/matrix_transformation_worker.h"
#include "operator_matrix.h"

#include <memory>
#include <utility>

namespace Moment {

    /**
     * Utility class, applies an element-wise transformation to operator matrix, with optional multithreading.
     * @tparam functor_t The transformation function (OperatorSequence -> OperatorSequence) type.
     */
    template<typename functor_t>
    class OperatorMatrixTransformation {
    public:
        /** Function to apply to each operator sequence. */
        const functor_t functor;

        /** Do we multithread? */
        const Multithreading::MultiThreadPolicy mt_policy;

        /** If multithreading is optional, what is the minimum element count to switch? */
        const size_t mt_difficulty_threshold;

    public:
        /**
         * Create an element-wise operator matrix transformer.
         * @param func The functor acts on each OperatorSequence.
         * @param mt_policy The multithreading policy (never, optional, always).
         * @param mt_difficulty The difficulty threshold, if multithreading is optional.
         */
        constexpr explicit OperatorMatrixTransformation(functor_t&& func,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional,
                      size_t mt_difficulty = std::numeric_limits<size_t>::max())
            : functor{std::move(func)}, mt_policy{mt_policy}, mt_difficulty_threshold{mt_difficulty} {

        }

        /**
         * Produce a new OperatorMatrix as a result of applying the functor to each element in turn.
         * @param input The matrix to transform
         * @return Newly constructed matrix.
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix> operator()(const OperatorMatrix& input) const {
            const size_t numel = input.Dimension() * input.Dimension();
            if (Multithreading::should_multithread(this->mt_policy, mt_difficulty_threshold, numel)) {
                return transform_multithreaded(input);
            } else {
                return transform_singlethreaded(input);
            }
        }

        /**
         * Produce a new OperatorMatrix as a result of applying the functor to each element in turn.
         * Explicitly singlethreaded.
         * @param input The matrix to transform
         * @return Newly constructed matrix.
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix> transform_singlethreaded(const OperatorMatrix& input) const {
            const size_t dimension = input.Dimension();
            const size_t numel = dimension * dimension;
            OperatorSequence const * const input_raw = input.raw();

            // Prepare output data for sequential write
            std::vector<OperatorSequence> output_data;
            output_data.reserve(numel);

            // Apply transformation
            std::transform(input_raw, input_raw+numel, std::back_inserter(output_data), this->functor);

            // Make operator matrix object
            return std::make_unique<OperatorMatrix>(input.context, dimension, std::move(output_data));
        }


        /**
         * Produce a new OperatorMatrix as a result of applying the functor to each element in turn.
         * Explicitly multithreaded.
         * @param input The matrix to transform
         * @return Newly constructed matrix.
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix> transform_multithreaded(const OperatorMatrix& input) const  {
            const size_t dimension = input.Dimension();
            const size_t numel = dimension * dimension;
            OperatorSequence const * const input_raw = input.raw();

            // Prepare output data for random access write
            std::vector<OperatorSequence> output_data{OperatorSequence::create_uninitialized_vector(numel)};

            // Apply transformation
            Multithreading::transform_matrix_data(dimension, input_raw, output_data.data(), this->functor);

            // Make operator matrix object
            return std::make_unique<OperatorMatrix>(input.context, dimension, std::move(output_data));
        }
    };

}