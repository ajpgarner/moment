/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"

#include "symbolic/monomial.h"
#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include <limits>
#include <stdexcept>

namespace Moment {

    namespace {
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        do_multiply_single_thread(const OperatorMatrix& lhs, const OperatorSequence& rhs) {
            std::vector<OperatorSequence> output_data;
            output_data.reserve(lhs.Dimension() * lhs.Dimension());
            for (const auto& lhs_elem : lhs()) {
                output_data.push_back(lhs_elem * rhs);
            }
            return std::make_unique<OperatorMatrix>(lhs.context,
                        std::make_unique<OperatorMatrix::OpSeqMatrix>(lhs.Dimension(), std::move(output_data)));
        }

        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        do_multiply_single_thread(const OperatorSequence& lhs, const OperatorMatrix& rhs) {
            std::vector<OperatorSequence> output_data;
            output_data.reserve(rhs.Dimension() * rhs.Dimension());
            for (const auto& rhs_elem : rhs()) {
                output_data.push_back(lhs * rhs_elem);
            }
            return std::make_unique<OperatorMatrix>(rhs.context,
                        std::make_unique<OperatorMatrix::OpSeqMatrix>(rhs.Dimension(), std::move(output_data)));
        }
    }

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

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::pre_multiply(const OperatorSequence& lhs, Multithreading::MultiThreadPolicy policy) const {
        assert(lhs.is_same_context(this->context));
        const bool should_mt = should_multithread_matrix_multiplication(policy, this->Dimension() * this->Dimension());
        return do_multiply_single_thread(lhs, *this);
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::post_multiply(const OperatorSequence& rhs, Multithreading::MultiThreadPolicy policy) const {
        assert(rhs.is_same_context(this->context));
        const bool should_mt = should_multithread_matrix_multiplication(policy, this->Dimension() * this->Dimension());
        return do_multiply_single_thread(*this, rhs);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::pre_multiply(const Polynomial &lhs, const SymbolTable &symbols,
                                 Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, lhs.size() * this->Dimension() * this->Dimension());

        std::vector<std::unique_ptr<OperatorMatrix>> output;
        for (const auto& monomial : lhs) {
            assert(monomial.id < symbols.size());
            const auto& lhs_symbol = symbols[monomial.id];
            assert(lhs_symbol.has_sequence());
            const auto& sequence = monomial.conjugated ? lhs_symbol.sequence_conj() : lhs_symbol.sequence();
            output.emplace_back(do_multiply_single_thread(sequence, *this));
        }
        return output;
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::post_multiply(const Polynomial &rhs, const SymbolTable &symbols,
                                  Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, rhs.size() * this->Dimension() * this->Dimension());

        std::vector<std::unique_ptr<OperatorMatrix>> output;
        for (const auto& monomial : rhs) {
            assert(monomial.id < symbols.size());
            const auto& rhs_symbol = symbols[monomial.id];
            assert(rhs_symbol.has_sequence());
            const auto& sequence = monomial.conjugated ? rhs_symbol.sequence_conj() : rhs_symbol.sequence();
            output.emplace_back(do_multiply_single_thread(*this, sequence));
        }
        return output;
    }

}