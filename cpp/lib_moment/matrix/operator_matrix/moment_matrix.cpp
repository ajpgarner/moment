/**
 * npa_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include "scenarios/context.h"

#include <limits>
#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment {
    namespace {
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
                    Multithreading::generate_matrix_data(colGen, rowGen,
                                                         raw_data.data(),
                                                         [&context](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                        return context.simplify_as_moment(lhs * rhs);
                    });
                } else {
                    Multithreading::generate_matrix_data(colGen, rowGen,
                                                         raw_data.data(),
                                                         [](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                                                             return lhs * rhs;
                                                         });
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