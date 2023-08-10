/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include <limits>
#include <stdexcept>

namespace Moment {

   OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)) {
       this->non_hermitian_elem = NonHInfo::find_first_index(*this);
       this->hermitian = !this->non_hermitian_elem.has_value();
    }

    OperatorMatrix::OpSeqMatrix::OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data,
                                             std::optional<NonHInfo> h_info)
        : SquareMatrix<OperatorSequence>(dimension, std::move(matrix_data)), non_hermitian_elem{std::move(h_info)} {
        this->hermitian = !non_hermitian_elem.has_value();
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