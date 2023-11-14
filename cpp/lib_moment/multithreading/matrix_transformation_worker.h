/**
 * matrix_transformation_worker.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading.h"

#include <algorithm>
#include <cassert>
#include <thread>
#include <vector>

namespace Moment::Multithreading {

    /**
     * Split matrix into rows, and transform.
     */
    template<typename input_elem_t, typename output_elem_t, typename elem_functor_t>
    class matrix_transformation_worker {
    private:
        std::thread the_thread;

    public:
        const size_t worker_id;
        const size_t max_workers;

        const input_elem_t *const input_ptr;
        output_elem_t *const output_ptr;

        const size_t dimension;

        const elem_functor_t &functor;

        matrix_transformation_worker(const size_t dimension,
                                 const input_elem_t *const input_data,
                                 output_elem_t *const output_data,
                                 const size_t worker_id,
                                 const size_t max_workers,
                                 const elem_functor_t &the_functor)
                : input_ptr{input_data}, output_ptr{output_data},
                  worker_id{worker_id}, max_workers{max_workers}, dimension{dimension},
                  functor{the_functor} {
            assert(worker_id < max_workers);
            assert(max_workers != 0);
            this->the_thread = std::thread(
                    &matrix_transformation_worker<input_elem_t, output_elem_t, elem_functor_t>::execute, this);
        }

        void execute() {
            for (size_t col_idx = worker_id; col_idx < dimension; col_idx += max_workers) {
                for (size_t row_idx = 0; row_idx < dimension; ++row_idx) {
                    const size_t offset = (col_idx*dimension) + row_idx;
                    output_ptr[offset] = functor(this->input_ptr[offset]);
                }
            }
        }

        void join() {
            this->the_thread.join();
        }
    };

    /** Create generation threads, run them, wait until they are finished. */
    template<typename input_elem_t, typename output_elem_t, typename elem_functor_t>
    void transform_matrix_data(const size_t dimension,
                               const input_elem_t * const input_data,
                               output_elem_t * const output_data,
                               const elem_functor_t &the_functor) {

        // Determine degree of parallelization
        const size_t num_threads = std::min(Multithreading::get_max_worker_threads(), dimension);

        using worker_t = matrix_transformation_worker<input_elem_t, output_elem_t, elem_functor_t>;

        // Create and start workers
        std::vector<worker_t> workers;
        workers.reserve(num_threads);
        for (size_t index = 0; index < num_threads; ++index) {
            workers.emplace_back(dimension, input_data, output_data, index, num_threads, the_functor);
        }

        // Wait for threads to finish...
        for (auto &worker: workers) {
            worker.join();
        }
    }
}