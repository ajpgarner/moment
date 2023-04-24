/**
 * npa_matrix.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix.h"
#include "moment_matrix_properties.h"

#include "operator_sequence_generator.h"

#include <limits>
#include <sstream>

namespace Moment {
    namespace {
        std::unique_ptr<OperatorMatrix::OpSeqMatrix>
        generate_moment_matrix_sequences(const Context& context, size_t level) {
            // Prepare generator of symbols
            const OperatorSequenceGenerator& colGen = context.operator_sequence_generator(level);
            const OperatorSequenceGenerator& rowGen = context.operator_sequence_generator(level, true);

            // Build matrix...
            size_t dimension = colGen.size();
            assert(dimension == rowGen.size());

            std::vector<OperatorSequence> matrix_data;
            matrix_data.reserve(dimension * dimension);
            for (const auto& rowSeq : rowGen) {
                for (const auto& colSeq : colGen) {
                    matrix_data.emplace_back(context.simplify_as_moment(rowSeq * colSeq));
                }
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

    MomentMatrix::MomentMatrix(const Context& context, size_t level)
        : OperatorMatrix{context, generate_moment_matrix_sequences(context, level)},
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

    const MomentMatrix* MomentMatrix::as_monomial_moment_matrix(const Matrix& input) noexcept {
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