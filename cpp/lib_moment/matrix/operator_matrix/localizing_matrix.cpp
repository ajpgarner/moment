/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include "scenarios/context.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include <sstream>

namespace Moment {
    namespace {
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
                if (context.can_have_aliases()) {
                    for (const auto &colSeq: colGen) {
                        for (const auto &rowSeq: rowGen) {
                            matrix_data.emplace_back(context.simplify_as_moment(rowSeq * (word * colSeq)));
                        }
                    }
                } else {
                    for (const auto &colSeq: colGen) {
                        for (const auto &rowSeq: rowGen) {
                            matrix_data.emplace_back(rowSeq * (word * colSeq));
                        }
                    }
                }

            } else {
                auto raw_data = OperatorSequence::create_uninitialized_vector(dimension*dimension);
                if (context.can_have_aliases()) {
                    Multithreading::generate_matrix_data(colGen, rowGen,
                                                         raw_data.data(),
                                                         [&context, &word](const OperatorSequence& lhs,
                                                                 const OperatorSequence& rhs) {
                                                             return context.simplify_as_moment(lhs * (word * rhs));
                                                         });
                } else {
                    Multithreading::generate_matrix_data(colGen, rowGen,
                                                         raw_data.data(),
                                                         [&word](const OperatorSequence& lhs,
                                                                           const OperatorSequence& rhs) {
                                                             return lhs * (word * rhs);
                                                         });
                }
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