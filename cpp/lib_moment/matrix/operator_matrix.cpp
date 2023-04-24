/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "operator_sequence_generator.h"

#include "matrix_properties.h"

#include <limits>
#include <stdexcept>
#include <sstream>

namespace Moment {

   OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)) {
        this->calculate_hermicity();
    }

    void OperatorMatrix::OpSeqMatrix::calculate_hermicity() {
        for (size_t row = 0; row < this->dimension; ++row) {
            if ((*this)[row][row] != (*this)[row][row].conjugate()) {
                this->hermitian = false;
                this->nonh_i = static_cast<ptrdiff_t>(row);
                this->nonh_j = static_cast<ptrdiff_t>(row);
                return;
            }
            for (size_t col = row+1; col < this->dimension; ++col) {
                const auto& upper = (*this)[row][col];
                const auto& lower = (*this)[col][row];
                const auto lower_conj = lower.conjugate();
                if (upper != lower_conj) {
                    this->hermitian = false;
                    this->nonh_i = static_cast<ptrdiff_t>(row);
                    this->nonh_j = static_cast<ptrdiff_t>(col);
                    return;
                }
            }
        }
        this->hermitian = true;
        this->nonh_i = this->nonh_j = -1;
    }

    OperatorMatrix::OperatorMatrix(const Context& context, std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
       : context{context},  op_seq_matrix{std::move(op_seq_mat)} {
           assert(op_seq_matrix);
       }

    OperatorMatrix::~OperatorMatrix() noexcept = default;
}