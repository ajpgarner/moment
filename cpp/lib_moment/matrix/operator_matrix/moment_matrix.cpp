/**
 * npa_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/multithreading.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {
    namespace {

        template<bool can_have_aliases>
        class moment_matrix_generation_worker {
        private:
            std::thread the_thread;
            const OperatorSequenceGenerator& colGen;
            const OperatorSequenceGenerator& rowGen;
        public:
            const size_t worker_id;
            const size_t max_workers;

            const Context& context;
            OperatorSequence * const base_ptr;
            const size_t row_length;

            moment_matrix_generation_worker(const Context& context,
                                            const OperatorSequenceGenerator& cols,
                                            const OperatorSequenceGenerator& rows,
                                            OperatorSequence* const output_data,
                                            const size_t worker_id, const size_t max_workers)
            : context{context}, colGen{cols}, rowGen{rows}, base_ptr{output_data},
                worker_id{worker_id}, max_workers{max_workers}, row_length{rowGen.size()} {
                assert(rowGen.size() == colGen.size());
                assert(worker_id < max_workers);
                assert(max_workers != 0);
                this->the_thread = std::thread(&moment_matrix_generation_worker::execute, this);
            }

            void execute() {
                for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                    const auto & colSeq = colGen[col_idx];
                    for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {
                        const auto& rowSeq = rowGen[row_idx];
                        size_t total_idx = (col_idx * row_length) + row_idx;
                        if constexpr(can_have_aliases) {
                            base_ptr[total_idx] = context.simplify_as_moment(rowSeq * colSeq);
                        } else {
                            base_ptr[total_idx] = rowSeq * colSeq;
                        }
                    }
                }
            }

            void join() {
                this->the_thread.join();
            }

            static void create_execute_and_wait(const Context& context,
                                                const OperatorSequenceGenerator& cols,
                                                const OperatorSequenceGenerator& rows,
                                                OperatorSequence* const output_data) {
                const size_t num_threads = Multithreading::get_max_worker_threads();
                std::vector<moment_matrix_generation_worker> workers;
                workers.reserve(num_threads);
                for (size_t index = 0; index < num_threads; ++index) {
                    workers.emplace_back(context, cols, rows, output_data, index, num_threads);
                }

                // Wait for threads to finish...
                for (auto& worker : workers) {
                    worker.join();
                }
            }
        };


        std::unique_ptr<OperatorMatrix::OpSeqMatrix>
        generate_moment_matrix_sequences(const Context& context, size_t level, const Multithreading::MultiThreadPolicy mt_policy) {
            // Prepare generator of symbols
            const OperatorSequenceGenerator& colGen = context.operator_sequence_generator(level);
            const OperatorSequenceGenerator& rowGen = context.operator_sequence_generator(level, true);

            // Build matrix...
            size_t dimension = colGen.size();
            assert(dimension == rowGen.size());

            const bool use_multithreading
                = Multithreading::should_multithread_matrix_creation(mt_policy, dimension*dimension);

            std::vector<OperatorSequence> matrix_data;
            if (!use_multithreading) {
                matrix_data.reserve(dimension * dimension);
                if (context.can_have_aliases()) {
                    for (const auto &colSeq: colGen) {
                        for (const auto &rowSeq: rowGen) {
                            matrix_data.emplace_back(context.simplify_as_moment(rowSeq * colSeq));
                        }
                    }
                } else {
                    for (const auto &colSeq: colGen) {
                        for (const auto &rowSeq: rowGen) {
                            matrix_data.emplace_back(rowSeq * colSeq);
                        }
                    }
                }
            } else {
                auto raw_data = OperatorSequence::create_uninitialized_vector(dimension*dimension);
                if (context.can_have_aliases()) {
                    moment_matrix_generation_worker<true>::create_execute_and_wait(context, colGen, rowGen,
                                                                                   raw_data.data());
                } else {
                    moment_matrix_generation_worker<true>::create_execute_and_wait(context, colGen, rowGen,
                                                                                   raw_data.data());
                }
                matrix_data.swap(raw_data);
            }


            auto op_matrix = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));

            // Force MM to be Hermitian or throw exception
            if (!op_matrix->is_hermitian()) [[unlikely]] {
                const auto maybe_bad_index = op_matrix->nonhermitian_index();
                assert(maybe_bad_index.has_value());
                const auto& bad_index = maybe_bad_index.value();
                const std::array<size_t, 2> bad_tx_index{bad_index[1], bad_index[0]};


                std::stringstream ss;
                ss << "Generated moment matrix should be Hermitian, but element [" << bad_index[0] << "," << bad_index[1] << "]"
                   << " " << (*op_matrix)(bad_index) << " could not be established as the conjugate of element"
                   << " [" <<  bad_index[1] << "," <<  bad_index[0] << "] " << (*op_matrix)(bad_tx_index) << " (conjugate: "
                   << (*op_matrix)(bad_tx_index).conjugate() << ").";
                throw errors::hermitian_failure{ss.str()};
            }
            return op_matrix;
        }
    }

    MomentMatrix::MomentMatrix(const Context& context, const size_t level,
                               const Multithreading::MultiThreadPolicy mt_policy)
        : OperatorMatrix{context, generate_moment_matrix_sequences(context, level, mt_policy)},
          hierarchy_level{level} {

    }

    MomentMatrix::MomentMatrix(MomentMatrix &&src) noexcept :
            OperatorMatrix{static_cast<OperatorMatrix&&>(src)},
            hierarchy_level{src.hierarchy_level} {
    }

    MomentMatrix::~MomentMatrix() noexcept = default;

    const OperatorSequenceGenerator& MomentMatrix::Generators() const {
        return this->context.operator_sequence_generator(this->Level());
    }

    std::string MomentMatrix::description() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->hierarchy_level;
        return ss.str();
    }

    const MomentMatrix* MomentMatrix::as_monomial_moment_matrix_ptr(const Matrix& input) noexcept {
        if (!input.is_monomial()) {
            return nullptr;
        }

        if (!input.has_operator_matrix()) {
            return nullptr;
        }

        const auto &op_matrix = input.operator_matrix();
        return dynamic_cast<const MomentMatrix *>(&op_matrix); // might be nullptr!
    }

}