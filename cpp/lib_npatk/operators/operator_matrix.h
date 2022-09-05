/**
 * operator_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "utilities/square_matrix.h"
#include "operator_sequence.h"
#include "symbolic/symbol_expression.h"
#include "context.h"
#include "symbol_table.h"

#include <map>
#include <memory>
#include <set>
#include <vector>
#include <cassert>
#include <atomic>
#include <span>


namespace NPATK {

    class SymbolTable;

    /** Matrix type */
    enum class MatrixType {
        Unknown = 0,
        /** Real-valued, matrix is symmetric */
        Symmetric = 1,
        /** Complex-valued, matrix is hermitian */
        Hermitian = 2
    };

    class OperatorMatrix {
    public:
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

        class SymbolMatrixProperties {
        private:
            const OperatorMatrix& matrix;
            std::set<symbol_name_t> included_symbols;
            std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> elem_keys;
            std::vector<symbol_name_t> real_entries;
            std::vector<symbol_name_t> imaginary_entries;
            MatrixType basis_type = MatrixType::Unknown;


        public:
            SymbolMatrixProperties(const OperatorMatrix& matrix,
                                   const SymbolTable& table,
                                   std::set<symbol_name_t>&& subset);

            [[nodiscard]] constexpr const auto& RealSymbols() const noexcept {
                return this->real_entries;
            }

            [[nodiscard]] constexpr const auto& ImaginarySymbols() const noexcept {
                return this->imaginary_entries;
            }

            [[nodiscard]] constexpr const auto& BasisMap() const noexcept {
                return this->elem_keys;
            }

            [[nodiscard]] std::pair<ptrdiff_t, ptrdiff_t> BasisKey(symbol_name_t id) const {
                auto iter = this->elem_keys.find(id);
                if (iter == this->elem_keys.cend()) {
                    return {-1, -1};
                }
                return iter->second;
            }

            [[nodiscard]] constexpr MatrixType Type() const noexcept {
                return this->basis_type;
            }

            friend class NPATK::OperatorMatrix;
        };

    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;

    protected:
        /* Look-up key for symbols */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** Matrix, as operator sequences */
        std::unique_ptr<SquareMatrix<OperatorSequence>> op_seq_matrix;

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
        explicit OperatorMatrix(const Context& context, SymbolTable& symbols) :
            context{context}, symbol_table{symbols}, Symbols{symbols},
            SymbolMatrix{*this}, SequenceMatrix{*this} { }

        OperatorMatrix(OperatorMatrix&& rhs) noexcept
            : context{rhs.context}, symbol_table{rhs.symbol_table}, dimension{rhs.dimension},
            op_seq_matrix{std::move(rhs.op_seq_matrix)}, sym_exp_matrix{std::move(rhs.sym_exp_matrix)},
            sym_mat_prop{std::move(rhs.sym_mat_prop)}, Symbols{rhs.Symbols},
            SymbolMatrix{*this}, SequenceMatrix{*this} { }

        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        const SymbolMatrixProperties& SMP() const noexcept {
            assert(this->sym_mat_prop);
            return *this->sym_mat_prop;
        }


    };
}