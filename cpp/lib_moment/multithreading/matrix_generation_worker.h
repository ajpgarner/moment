/**
 * matrix_creation_worker.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence_generator.h"
#include "multithreading.h"

#include <thread>
#include <vector>

namespace Moment::Multithreading {

    template<typename elem_functor_t>
    class matrix_generation_worker {
    private:
        std::thread the_thread;
        const OperatorSequenceGenerator &colGen;
        const OperatorSequenceGenerator &rowGen;

    public:
        const size_t worker_id;
        const size_t max_workers;

        const Context &context;
        OperatorSequence * const base_ptr;
        const size_t row_length;

        const elem_functor_t &functor;

        matrix_generation_worker(const Context &context,
                                 const OperatorSequenceGenerator &cols,
                                 const OperatorSequenceGenerator &rows,
                                 OperatorSequence *const output_data,
                                 const size_t worker_id,
                                 const size_t max_workers,
                                 const elem_functor_t &the_functor)
                : context{context}, colGen{cols}, rowGen{rows}, base_ptr{output_data},
                  worker_id{worker_id}, max_workers{max_workers}, row_length{rowGen.size()},
                  functor{the_functor} {
            assert(rowGen.size() == colGen.size());
            assert(worker_id < max_workers);
            assert(max_workers != 0);
            this->the_thread = std::thread(&matrix_generation_worker<elem_functor_t>::execute, this);
        }

        void execute() {
            for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                const auto &colSeq = colGen[col_idx];
                for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {
                    const auto &rowSeq = rowGen[row_idx];
                    size_t total_idx = (col_idx * row_length) + row_idx;
                    base_ptr[total_idx] = functor(rowSeq, colSeq);
                }
            }
        }

        void join() {
            this->the_thread.join();
        }
    };

    /** Create generation threads, run them, wait until they are finished. */
    template<typename elem_functor_t>
    void generate_matrix_data(const Context& context,
                              const OperatorSequenceGenerator& cols, const OperatorSequenceGenerator& rows,
                              OperatorSequence* const output_data,
                              const elem_functor_t& the_functor) {
        const size_t num_threads = Multithreading::get_max_worker_threads();

        std::vector<matrix_generation_worker<elem_functor_t>> workers;
        workers.reserve(num_threads);
        for (size_t index = 0; index < num_threads; ++index) {
            workers.emplace_back(context, cols, rows, output_data, index, num_threads, the_functor);
        }

        // Wait for threads to finish...
        for (auto& worker : workers) {
            worker.join();
        }
    }

}