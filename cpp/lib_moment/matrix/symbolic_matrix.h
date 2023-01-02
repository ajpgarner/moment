/**
 * symbol_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol_table.h"
#include "utilities/square_matrix.h"

#include "matrix_properties.h"

#include <memory>

namespace Moment {

    class SymbolTable;
    class MatrixProperties;

    class SymbolicMatrix {
    public:
        class SymbolMatrixView {
        private:
            const SymbolicMatrix& matrix;
        public:
            explicit SymbolMatrixView(const SymbolicMatrix& theMatrix) noexcept : matrix{theMatrix} { };

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

    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;

    protected:
        /** Look-up key for symbols */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** True, if Hermitian */
        bool is_hermitian = false;

        /** Matrix, as symbolic expression */
        std::unique_ptr<SquareMatrix<SymbolExpression>> sym_exp_matrix;

        /** Symbol matrix properties (basis size, etc.) */
        std::unique_ptr<MatrixProperties> mat_prop;

    public:
        /**
         * Table of symbols for entire system.
         */
        const SymbolTable& Symbols;

        /**
         * Matrix, as symbols
         */
        SymbolMatrixView SymbolMatrix;

    protected:
        SymbolicMatrix(const Context& context, SymbolTable& symbols,
                       std::unique_ptr<SquareMatrix<SymbolExpression>> symbolMatrix);

        SymbolicMatrix(SymbolicMatrix&& rhs)
            : context{rhs.context}, symbol_table{rhs.symbol_table}, dimension{rhs.dimension},
              is_hermitian{rhs.is_hermitian}, sym_exp_matrix{std::move(rhs.sym_exp_matrix)},
              mat_prop{std::move(rhs.mat_prop)}, Symbols{rhs.Symbols}, SymbolMatrix{*this} { }

    public:
        virtual ~SymbolicMatrix() noexcept;

        /**
         * Dimension of the matrix
         */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /**
         * True, if matrix is Hermitian
         */
        [[nodiscard]] constexpr bool IsHermitian() const noexcept {
            return this->is_hermitian;
        }

        /**
         * Properties of the matrix
         */
        [[nodiscard]] const MatrixProperties& SMP() const noexcept {
            assert(this->mat_prop);
            return *this->mat_prop;
        }
    };
}