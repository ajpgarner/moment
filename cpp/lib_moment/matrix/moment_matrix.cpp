/**
 * npa_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"
#include "properties/moment_matrix_properties.h"

#include "operator_sequence_generator.h"

#include "utilities/multithreading.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {
    namespace {

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
                for (size_t row_idx = worker_id; row_idx < row_length; row_idx += max_workers) {
                    const auto& rowSeq = rowGen[row_idx];
                    for (size_t col_idx = 0; col_idx < row_length; ++col_idx) {
                        size_t total_idx = (row_idx * row_length) + col_idx;
                        const auto & colSeq = colGen[col_idx];

                        base_ptr[total_idx] = context.simplify_as_moment(rowSeq * colSeq);
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

            const bool use_multithreading = Multithreading::should_multithread_matrix_creation(mt_policy,
                                                                                               dimension*dimension);

            std::vector<OperatorSequence> matrix_data;
            if (!use_multithreading) {
                matrix_data.reserve(dimension * dimension);
                for (const auto &rowSeq: rowGen) {
                    for (const auto &colSeq: colGen) {
                        matrix_data.emplace_back(context.simplify_as_moment(rowSeq * colSeq));
                    }
                }
            } else {
                auto raw_data = OperatorSequence::create_uninitialized_vector(dimension*dimension);
                moment_matrix_generation_worker::create_execute_and_wait(context, colGen, rowGen, raw_data.data());
                matrix_data.swap(raw_data);
               // throw std::logic_error("Multithreading not supported.");
            }


            auto op_matrix = std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));

            // Force MM to be Hermitian or throw exception
            if (!op_matrix->is_hermitian()) {
                auto [bad_row, bad_col] = op_matrix->nonhermitian_index();
                assert(bad_row >= 0);
                assert(bad_col >= 0);

                std::stringstream ss;
                ss << "Generated moment matrix should be Hermitian, but element [" << bad_row << "," << bad_col << "]"
                   << " " << (*op_matrix)[bad_row][bad_col] << " could not be established as the conjugate of element"
                   << " [" << bad_col << "," << bad_row << "] " << (*op_matrix)[bad_col][bad_row] << " (conjugate: "
                   << (*op_matrix)[bad_col][bad_row].conjugate() << ").";
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

    std::unique_ptr<MatrixProperties> MomentMatrix::replace_properties(std::unique_ptr<MatrixProperties> input) const {
        return std::make_unique<MomentMatrixProperties>(std::move(*input),
                                                        this->hierarchy_level, this->op_seq_matrix->is_hermitian(),
                                                        this->description());
    }

}