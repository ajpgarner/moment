/**
 * localizing_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix.h"

#include "operator_sequence_generator.h"

#include <sstream>

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

    std::string LocalizingMatrix::description() const {
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << this->Index.Level << ", Word " << this->Index.Word;
        return ss.str();
    }

}