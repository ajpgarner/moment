/**
 * npa_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "moment_matrix.h"

#include "../context.h"

#include "../operator_sequence_generator.h"
#include "operator_matrix.h"


#include <limits>


namespace NPATK {
    namespace {
        std::unique_ptr<SquareMatrix<OperatorSequence>>
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
                    matrix_data.emplace_back(rowSeq * colSeq);
                }
            }

            return std::make_unique<SquareMatrix<OperatorSequence>>(dimension, std::move(matrix_data));
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

}