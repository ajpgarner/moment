/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "operator_matrix_creation_context.h"

#include "dictionary/operator_sequence_generator.h"

#include <limits>
#include <stdexcept>

namespace Moment {

   OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)) {
        this->calculate_hermicity();
    }

    void OperatorMatrix::OpSeqMatrix::calculate_hermicity() {
        for (size_t col = 0; col < this->dimension; ++col) {
            auto& diag_elem = (*this)(std::array<size_t,2>{col, col});

            if (diag_elem != diag_elem.conjugate()) {
                this->hermitian = false;
                this->nonh_i = static_cast<ptrdiff_t>(col);
                this->nonh_j = static_cast<ptrdiff_t>(col);
                return;
            }
            for (size_t row = col+1; row < this->dimension; ++row) {

                const auto& upper = (*this)(std::array<size_t, 2>{row, col});
                const auto& lower = (*this)(std::array<size_t, 2>{col, row});
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

    void OperatorMatrix::set_properties(SymbolicMatrix &matrix) const {
        assert(matrix.op_mat && (matrix.op_mat.get() == this));
        matrix.description = this->description();
        matrix.hermitian = this->is_hermitian();
    }
}