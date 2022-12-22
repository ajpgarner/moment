/**
 * localizing_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "localizing_matrix.h"

#include "operator_sequence_generator.h"

namespace Moment {
    namespace {

        std::unique_ptr<OperatorMatrix::OpSeqMatrix>
        generate_localizing_matrix_sequences(const Context& context, size_t level, const OperatorSequence& word) {
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
                    matrix_data.emplace_back(context.simplify_as_moment(rowSeq * (word * colSeq)));
                }
            }
            return std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(matrix_data));
        }
    }


    LocalizingMatrix::LocalizingMatrix(const Context& context, SymbolTable& symbols, LocalizingMatrixIndex lmi)
        : OperatorMatrix{context, symbols, generate_localizing_matrix_sequences(context, lmi.Level, lmi.Word)},
          Index{std::move(lmi)} {

    }

}