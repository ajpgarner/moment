/**
 * operator_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "utilities/square_matrix.h"
#include "symbolic/symbol_expression.h"

#include "operators/operator_sequence.h"
#include "operators/context.h"

#include "symbol_table.h"
#include "symbol_matrix_properties.h"

#include <memory>
#include <cassert>
#include <map>
#include <span>

namespace NPATK {

    class SymbolTable;
    class SymbolMatrixProperties;


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
             * Get first row and column indices of non-hermitian element in matrix, if any. Otherwise, (-1,-1).
             */
            [[nodiscard]] std::pair<ptrdiff_t, ptrdiff_t> nonhermitian_index() const noexcept {
                return {nonh_i, nonh_j};
            }

        private:
            void calculate_hermicity();
        };

        class SymbolMatrixView {
        private:
            const OperatorMatrix& matrix;
        public:
            explicit SymbolMatrixView(const OperatorMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

            /**
            * Return a view (std::span<const SymbolExpression>) to the requested row of the NPA matrix's symbolic
            * representation. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const SymbolExpression> of the requested row.
            */
            std::span<const SymbolExpression> operator[](size_t row) const noexcept {
                return (*(matrix.sym_exp_matrix))[row];
            };

            /**
             * Provides access to square matrix of symbols.
             */
            const auto& operator()() const noexcept {
                return (*(matrix.sym_exp_matrix));
            }

        };

        class SequenceMatrixView {
        private:
            const OperatorMatrix& matrix;
        public:
            explicit SequenceMatrixView(const OperatorMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

            /**
            * Return a view (std::span<const OperatorSequence>) to the requested row of the NPA matrix's operator
            * sequences. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const OperatorSequence> of the requested row.
            */
            std::span<const OperatorSequence> operator[](size_t row) const noexcept {
                return (*(matrix.op_seq_matrix))[row];
            };

            /**
             * Provides access to square matrix of operator sequences.
             */
            const auto& operator()() const noexcept {
                return (*(matrix.op_seq_matrix));
            }

        };


    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;


    protected:
        /* Look-up key for symbols */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** True, if Hermitian */
        bool is_hermitian = false;

        /** Matrix, as operator sequences */
        std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_matrix;

        /** Matrix, as hashes */
        std::unique_ptr<SquareMatrix<uint64_t>> hash_matrix;

        /** Matrix, as symbolic expression */
        std::unique_ptr<SquareMatrix<SymbolExpression>> sym_exp_matrix;

        /** Symbol matrix properties (basis size, etc.) */
        std::unique_ptr<SymbolMatrixProperties> sym_mat_prop;

    public:
        /**
         * Table of symbols for entire system.
         */
        const SymbolTable& Symbols;

        /**
         * Matrix, as symbols
         */
        SymbolMatrixView SymbolMatrix;

        /**
         * Matrix, as operator strings
         */
        SequenceMatrixView SequenceMatrix;

    public:
        explicit OperatorMatrix(const Context& context, SymbolTable& symbols,
                                std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

        OperatorMatrix(OperatorMatrix&& rhs) noexcept
            : context{rhs.context}, symbol_table{rhs.symbol_table},
            dimension{rhs.dimension}, is_hermitian{rhs.is_hermitian},
            op_seq_matrix{std::move(rhs.op_seq_matrix)},
            sym_exp_matrix{std::move(rhs.sym_exp_matrix)},
            sym_mat_prop{std::move(rhs.sym_mat_prop)},
            Symbols{rhs.Symbols}, SymbolMatrix{*this}, SequenceMatrix{*this} { }

        virtual ~OperatorMatrix() noexcept;

        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        [[nodiscard]] constexpr bool IsHermitian() const noexcept {
            return this->is_hermitian;
        }

        [[nodiscard]] const SymbolMatrixProperties& SMP() const noexcept {
            assert(this->sym_mat_prop);
            return *this->sym_mat_prop;
        }

    private:
        std::set<symbol_name_t> integrateSymbols();

        std::vector<UniqueSequence> identifyUniqueSequences();
        std::vector<UniqueSequence> identifyUniqueSequencesHermitian();
        std::vector<UniqueSequence> identifyUniqueSequencesGeneric();

        std::unique_ptr<SquareMatrix<SymbolExpression>> buildSymbolMatrix();
        std::unique_ptr<SquareMatrix<SymbolExpression>> buildSymbolMatrixHermitian();
        std::unique_ptr<SquareMatrix<SymbolExpression>> buildSymbolMatrixGeneric();

    };
}