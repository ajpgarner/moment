/**
 * npa_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "npa_matrix.h"
#include "context.h"
#include "operator_sequence_generator.h"


namespace NPATK {

    NPAMatrix::NPAMatrix(const Context &the_context, size_t level)
        : context{the_context}, hierarchy_level{level} {
        OperatorSequenceGenerator colGen{context, hierarchy_level};
        OperatorSequenceGenerator rowGen{colGen.conjugate()};
        this->matrix_dimension = colGen.size();
        assert(this->matrix_dimension == rowGen.size());

        // Build matrix...
        this->matrix_data.reserve(this->matrix_dimension * this->matrix_dimension);
        for (const auto& rowSeq : rowGen) {
            for (const auto& colSeq : colGen) {
                this->matrix_data.emplace_back(rowSeq * colSeq);
            }
        }
    }
}