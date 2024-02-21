/**
 * operator_matrix.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/monomial_matrix.h"

#include "scenarios/context.h"

#include "dictionary/operator_sequence.h"
#include "dictionary/osg_pair.h"

#include "multithreading/multithreading.h"

#include "symbolic/monomial.h"
#include "symbolic/symbol_table.h"

#include "tensor/square_matrix.h"

#include "is_hermitian.h"

#include <cassert>

#include <map>
#include <memory>
#include <optional>
#include <span>

namespace Moment {

    class Polynomial;
    class SymbolTable;
    class SymbolMatrix;

    class OperatorMatrix {
    public:
        /**
         * Extend square matrix with Hermicity tests.
         */
        class OpSeqMatrix : public SquareMatrix<OperatorSequence> {
        private:
            bool hermitian = false;
            std::optional<NonHInfo> non_hermitian_elem;

        public:
            OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data);

            OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data, std::optional<NonHInfo> h_info);

            /**
             * True if the matrix is Hermitian.
             */
            [[nodiscard]] bool is_hermitian() const noexcept { return this->hermitian; }

            /**
             * Get first row and column indices of non-hermitian element in matrix, if any. Otherwise nullopt.
             */
            [[nodiscard]] std::optional<Index> nonhermitian_index() const noexcept {
                if (non_hermitian_elem.has_value()) {
                    return non_hermitian_elem->Index;
                } else {
                    return std::nullopt;
                }
            }
        };


    private:
        /** Matrix, as operator sequences */
        std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_matrix;

    public:
        const Context& context;

    public:
        explicit OperatorMatrix(const Context& context, std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

        OperatorMatrix(OperatorMatrix&& rhs) noexcept = default;

        virtual ~OperatorMatrix() noexcept;

        [[nodiscard]] size_t Dimension() const noexcept { return this->op_seq_matrix->dimension; }

        [[nodiscard]] bool is_hermitian() const noexcept { return this->op_seq_matrix->is_hermitian(); }

        /**
          * Return a view (std::span<const OperatorSequence>) to the requested row of the NPA matrix's operator
          * sequences. Since std::span also provides an operator[], it is possible to index using
          * "mySMV[row][col]" notation.
          * @param row The index of the row to return.
          * @return A std::span<const OperatorSequence> of the requested row.
          */
        const OperatorSequence&
        operator()(SquareMatrix<OperatorSequence>::IndexView index) const noexcept(!debug_mode) {
            return (*(this->op_seq_matrix))(index);
        };

        /**
         * Convenience indexing..
         */
        const OperatorSequence&
        operator()(size_t row, size_t col) const noexcept(!debug_mode) {
            return (*(this->op_seq_matrix))(SquareMatrix<OperatorSequence>::Index{row, col});
        };

        /**
         * Provides direct access to square matrix of operator sequences.
         */
        const auto& operator()() const noexcept {
            return (*(this->op_seq_matrix));
        }

        /**
         * Provide extremely raw access to operator sequences.
         */
         [[nodiscard]] const OperatorSequence* raw() const noexcept {
             return this->op_seq_matrix->raw();
         }

        /**
         * Operator matrices usually are made in an algorithmic manner, and can provide a name to their symbolization.
         */
        [[nodiscard]] virtual std::string description() const {
            return "Operator Matrix";
        }

        /**
         * Provide access to matrix generators
         */
         [[nodiscard]] virtual const OSGPair& generators() const;


        /** Apply the properties from this operator matrix to the supplied matrix. */
        void set_properties(SymbolicMatrix& matrix) const;

        /** Create a new operator matrix by pre-multiplying by an operator sequence */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        pre_multiply(const OperatorSequence& lhs,
                    Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /** Create a new operator matrix by post-multiplying by an operator sequence */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        post_multiply(const OperatorSequence& rhs,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of raw polynomial, by pre-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        pre_multiply(const RawPolynomial& lhs,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of raw polynomial, by post-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        post_multiply(const RawPolynomial& rhs,
                      Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of polynomial, by pre-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        pre_multiply(const Polynomial& lhs, const SymbolTable& symbols,
                     Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Create a new operator matrix per element of polynomial, by post-multiplying.
         * This will ignore factors!
         */
        [[nodiscard]] std::vector<std::unique_ptr<OperatorMatrix>>
        post_multiply(const Polynomial& rhs, const SymbolTable& symbols,
                      Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        /**
         * Creates a copy of this matrix
         */
        [[nodiscard]] std::unique_ptr<OperatorMatrix>
        clone(Multithreading::MultiThreadPolicy policy = Multithreading::MultiThreadPolicy::Optional) const;

        friend class SymbolicMatrix;
        friend class MonomialMatrix;
        friend class PolynomialMatrix;
    };
}