/**
 * operator_matrix.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_matrix.h"

#include "dictionary/operator_sequence_generator.h"
#include "dictionary/raw_polynomial.h"

#include "symbolic/monomial.h"
#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include <limits>
#include <stdexcept>

namespace Moment {

    namespace {
        template<bool premultiply>
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        inline do_multiply_single_thread(const OperatorMatrix& the_matrix, const OperatorSequence& the_sequence) {
            std::vector<OperatorSequence> output_data;
            const auto dimension = the_matrix.Dimension();
            output_data.reserve(dimension * dimension);
            for (const auto& matrix_elem : the_matrix()) {
                if constexpr (premultiply) {
                    output_data.push_back(the_sequence * matrix_elem);
                } else {
                    output_data.push_back(matrix_elem * the_sequence);
                }
            }
            return std::make_unique<OperatorMatrix>(the_matrix.context,
                        std::make_unique<OperatorMatrix::OpSeqMatrix>(dimension, std::move(output_data)));
        }

        template<bool premultiply>
        inline std::vector<std::unique_ptr<OperatorMatrix>>
        do_raw_poly_multiply_single_thread(const OperatorMatrix& the_matrix, const RawPolynomial& the_poly)  {
            std::vector<std::unique_ptr<OperatorMatrix>> output;
            for (const auto& monomial : the_poly) {
                output.emplace_back(do_multiply_single_thread<premultiply>(the_matrix, monomial.sequence));
            }
            return output;
        }

        template<bool premultiply>
        inline std::vector<std::unique_ptr<OperatorMatrix>>
        do_poly_multiply_single_thread(const OperatorMatrix& the_matrix, const Polynomial& the_poly,
                                       const SymbolTable &symbols)  {
            std::vector<std::unique_ptr<OperatorMatrix>> output;
            for (const auto& monomial : the_poly) {
                assert(monomial.id < symbols.size());
                const auto& symbolic_resolution = symbols[monomial.id];
                assert(symbolic_resolution.has_sequence());
                const auto& sequence = monomial.conjugated ? symbolic_resolution.sequence_conj()
                                                           : symbolic_resolution.sequence();
                output.emplace_back(do_multiply_single_thread<premultiply>(the_matrix, sequence));
            }
            return output;
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

    const OSGPair& OperatorMatrix::generators() const {
        throw std::runtime_error{"Generic OperatorMatrix does not have any attached generators."};
    }

    void OperatorMatrix::set_properties(SymbolicMatrix &matrix) const {
        assert(matrix.op_mat && (matrix.op_mat.get() == this));
        matrix.description = this->description();
        matrix.hermitian = this->is_hermitian();
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::pre_multiply(const OperatorSequence& lhs, Multithreading::MultiThreadPolicy policy) const {
        assert(lhs.is_same_context(this->context));
        const bool should_mt = should_multithread_matrix_multiplication(policy, this->Dimension() * this->Dimension());

        return do_multiply_single_thread<true>(*this, lhs);
    }

    std::unique_ptr<OperatorMatrix>
    OperatorMatrix::post_multiply(const OperatorSequence& rhs, Multithreading::MultiThreadPolicy policy) const {
        assert(rhs.is_same_context(this->context));
        const bool should_mt = should_multithread_matrix_multiplication(policy, this->Dimension() * this->Dimension());

        return do_multiply_single_thread<false>(*this, rhs);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::pre_multiply(const RawPolynomial& lhs, Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, lhs.size() * this->Dimension() * this->Dimension());

        return do_raw_poly_multiply_single_thread<true>(*this, lhs);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::post_multiply(const RawPolynomial& rhs, Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, rhs.size() * this->Dimension() * this->Dimension());

        return do_raw_poly_multiply_single_thread<false>(*this, rhs);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::pre_multiply(const Polynomial &lhs, const SymbolTable &symbols,
                                 Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, lhs.size() * this->Dimension() * this->Dimension());

        return do_poly_multiply_single_thread<true>(*this, lhs, symbols);
    }

    std::vector<std::unique_ptr<OperatorMatrix>>
    OperatorMatrix::post_multiply(const Polynomial &rhs, const SymbolTable &symbols,
                                  Multithreading::MultiThreadPolicy policy) const {
        const bool should_mt =
                should_multithread_matrix_multiplication(policy, rhs.size() * this->Dimension() * this->Dimension());

        return do_poly_multiply_single_thread<false>(*this, rhs, symbols);
    }

}