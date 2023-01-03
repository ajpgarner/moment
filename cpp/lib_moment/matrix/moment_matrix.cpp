/**
 * npa_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "moment_matrix.h"

#include "operator_sequence_generator.h"

#include <limits>
#include <sstream>

namespace Moment {
    namespace {
        std::unique_ptr<OperatorMatrix::OpSeqMatrix>
        generate_moment_matrix_sequences(const Context& context, size_t level) {
            // Prepare generator of symbols
            OperatorSequenceGenerator colGen{context, level};
            OperatorSequenceGenerator rowGen{colGen.conjugate()};

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

    MomentMatrix::MomentMatrix(const Context& context, SymbolTable& symbols, size_t level)
        : OperatorMatrix{context, symbols, generate_moment_matrix_sequences(context, level)},
          hierarchy_level{level} {
    }

    MomentMatrix::MomentMatrix(MomentMatrix &&src) noexcept :
            OperatorMatrix{static_cast<OperatorMatrix&&>(src)},
            hierarchy_level{src.hierarchy_level} {

    }

    MomentMatrix::~MomentMatrix() = default;

    OperatorSequenceGenerator MomentMatrix::Generators() const {
        return OperatorSequenceGenerator{this->context, this->Level()};
    }

}