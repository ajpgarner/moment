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

#include "symbolic/monomial.h"
#include "symbolic/symbol_table.h"

#include "tensor/square_matrix.h"

#include <cassert>

#include <map>
#include <memory>
#include <optional>
#include <span>

namespace Moment {

    class SymbolTable;
    class SymbolMatrix;

    class OperatorMatrix {
    public:
        class OpSeqMatrix : public SquareMatrix<OperatorSequence> {
        private:
            bool hermitian = false;
            ptrdiff_t nonh_i = -1;
            ptrdiff_t nonh_j = -1;
        public:

            OpSeqMatrix(size_t dimension, std::vector<OperatorSequence> matrix_data);

            /**
             * True if the matrix is Hermitian.
             */
            [[nodiscard]] bool is_hermitian() const noexcept { return this->hermitian; }

            /**
             * Get first row and column indices of non-hermitian element in matrix, if any. Otherwise nullopt.
             */
            [[nodiscard]] std::optional<Index> nonhermitian_index() const noexcept {
                if (nonh_i == -1) {
                    assert(nonh_j == -1);
                    return std::nullopt;
                }
                assert(nonh_j != -1);
                return Index{static_cast<size_t>(nonh_i), static_cast<size_t>(nonh_j)};
            }

        private:
            void calculate_hermicity();
        };


    protected:
        /** Matrix, as operator sequences */
        std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_matrix;

        /** Matrix, as hashes */
        std::unique_ptr<SquareMatrix<uint64_t>> hash_matrix;

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

        [[nodiscard]] virtual std::string description() const {
            return "Operator Matrix";
        }

        /** Apply the properties from this operator matrix to the supplied matrix. */
        void set_properties(SymbolicMatrix& matrix) const;

    };
}