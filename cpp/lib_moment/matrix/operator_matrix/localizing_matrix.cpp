/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include <sstream>

namespace Moment {
    namespace {

        class localizing_matrix_generation_worker {
        private:
            std::thread the_thread;
            const OperatorSequence word;
            const OperatorSequenceGenerator& colGen;
            const OperatorSequenceGenerator& rowGen;

        public:
            const size_t worker_id;
            const size_t max_workers;

            const Context& context;
            OperatorSequence * const base_ptr;
            const size_t row_length;

            localizing_matrix_generation_worker(const Context& context,
                                            const OperatorSequence& word,
                                            const OperatorSequenceGenerator& cols,
                                            const OperatorSequenceGenerator& rows,
                                            OperatorSequence* const output_data,
                                            const size_t worker_id, const size_t max_workers)
                    : context{context}, word{word}, colGen{cols}, rowGen{rows}, base_ptr{output_data},
                      worker_id{worker_id}, max_workers{max_workers}, row_length{rowGen.size()} {
                assert(rowGen.size() == colGen.size());
                assert(worker_id < max_workers);
                assert(max_workers != 0);
                this->the_thread = std::thread(&localizing_matrix_generation_worker::execute, this);
            }

            void execute() {
                for (size_t col_idx = worker_id; col_idx < row_length; col_idx += max_workers) {
                    const auto & colSeq = colGen[col_idx];
                    for (size_t row_idx = 0; row_idx < row_length; ++row_idx) {
                        const auto& rowSeq = rowGen[row_idx];
                        size_t total_idx = (col_idx * row_length) + row_idx;
                        base_ptr[total_idx] = context.simplify_as_moment(rowSeq * (word * colSeq));
                    }
                }
            }

            void join() {
                this->the_thread.join();
            }

            static void create_execute_and_wait(const Context& context,
                                                const OperatorSequence& word,
                                                const OperatorSequenceGenerator& cols,
                                                const OperatorSequenceGenerator& rows,
                                                OperatorSequence* const output_data) {
                const size_t num_threads = Multithreading::get_max_worker_threads();
                std::vector<localizing_matrix_generation_worker> workers;
                workers.reserve(num_threads);
                for (size_t index = 0; index < num_threads; ++index) {
                    workers.emplace_back(context, word, cols, rows, output_data, index, num_threads);
                }

                // Wait for threads to finish...
                for (auto& worker : workers) {
                    worker.join();
                }
            }
        };

        std::unique_ptr<OperatorMatrix::OpSeqMatrix>
        generate_localizing_matrix_sequences(const Context& context, size_t level,
                                             const OperatorSequence& word,
                                             Multithreading::MultiThreadPolicy mt_policy) {
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
                for (const auto &colSeq: colGen) {
                    for (const auto &rowSeq: rowGen) {
                        matrix_data.emplace_back(context.simplify_as_moment(rowSeq * (word * colSeq)));
                    }
                }
            } else {
                auto raw_data = OperatorSequence::create_uninitialized_vector(dimension*dimension);
                localizing_matrix_generation_worker::create_execute_and_wait(context, word, colGen, rowGen,
                                                                             raw_data.data());
                matrix_data.swap(raw_data);
            }

            return std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));
        }

        inline const Context& assert_context(const Context& context, const LocalizingMatrixIndex& lmi) {
            assert(lmi.Word.is_same_context(context));
            return context;
        }
    }


    LocalizingMatrix::LocalizingMatrix(const Context& context, LocalizingMatrixIndex lmi,
                                       Multithreading::MultiThreadPolicy mt_policy)
        : OperatorMatrix{assert_context(context, lmi),
                         generate_localizing_matrix_sequences(context, lmi.Level, lmi.Word, mt_policy)},
          Index{std::move(lmi)} {

    }

    LocalizingMatrix::~LocalizingMatrix() noexcept = default;

    std::string LocalizingMatrix::description() const {
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << this->Index.Level << ", Word " << this->Index.Word;
        return ss.str();
    }


    const LocalizingMatrix* LocalizingMatrix::as_monomial_localizing_matrix_ptr(const Matrix& input) noexcept {
        if (!input.is_monomial()) {
            return nullptr;
        }

        if (!input.has_operator_matrix()) {
            return nullptr;
        }

        const auto& op_matrix = input.operator_matrix();
        return dynamic_cast<const LocalizingMatrix*>(&op_matrix); // might be nullptr!
    }

//    std::unique_ptr<MatrixProperties>
//    LocalizingMatrix::replace_properties(std::unique_ptr<MatrixProperties> input) const {
//        return std::make_unique<LocalizingMatrixProperties>(std::move(*input), this->Index,
//                                                            this->op_seq_matrix->is_hermitian(),
//                                                            this->description());
//    }

}