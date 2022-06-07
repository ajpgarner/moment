/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"
#include "symbolic/symbol_expression.h"
#include "square_matrix.h"

#include <cassert>
#include <memory>
#include <span>
#include <map>

namespace NPATK {
    class Context;


    class NPAMatrix {
    private:
        class UniqueSequence {
            symbol_name_t id = -1;
            OperatorSequence opSeq;
            std::optional<OperatorSequence> conjSeq{};
            size_t fwd_hash = 0;
            size_t conj_hash = 0;
            bool hermitian = false;

            constexpr UniqueSequence(OperatorSequence sequence, size_t hash) :
                    opSeq{std::move(sequence)}, fwd_hash{hash},
                    conjSeq{}, conj_hash{hash},
                    hermitian{true} { }

            constexpr UniqueSequence(OperatorSequence sequence, size_t hash,
                           OperatorSequence conjSequence, size_t conjHash) :
                    opSeq{std::move(sequence)}, fwd_hash{hash},
                    conjSeq{std::move(conjSequence)}, conj_hash{conjHash},
                    hermitian{false} { }

        public:
            [[nodiscard]] constexpr size_t hash() const noexcept { return this->fwd_hash; }
            [[nodiscard]] constexpr size_t hash_conj() const noexcept { return this->conj_hash; }
            [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept { return this->opSeq; }
            [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
                return this->hermitian ? opSeq : this->conjSeq.value();
            }
            /**
             * Does the operator sequence represent its Hermitian conjugate?
             * If true, the element will correspond to a real symbol (cf. complex if not) in the NPA matrix.
             */
            [[nodiscard]] constexpr bool is_hermitian() const noexcept { return this->hermitian; }

            inline static UniqueSequence Zero(const Context& context) {
                return UniqueSequence{OperatorSequence::Zero(&context), 0};
            }

            inline static UniqueSequence Identity(const Context& context) {
                return UniqueSequence{OperatorSequence::Identity(&context), 1};
            }

            friend class NPAMatrix;
        };

        class SymbolMatrixView {
        private:
            const NPAMatrix& matrix;
        public:
            explicit SymbolMatrixView(const NPAMatrix& theMatrix) : matrix{theMatrix} { };
            [[nodiscard]] size_t dimension() const noexcept { return matrix.dimension(); }
            [[nodiscard]] std::pair<size_t, size_t> dimensions() const noexcept { return matrix.dimensions(); }

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

        class UniqueSequenceRange {
        private:
            const NPAMatrix& matrix;
        public:
            explicit UniqueSequenceRange(const NPAMatrix& theMatrix) : matrix{theMatrix} { }
            [[nodiscard]] auto begin() const noexcept { return matrix.unique_sequences.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return matrix.unique_sequences.cend(); }
            [[nodiscard]] bool empty() const noexcept { return matrix.unique_sequences.empty(); }
            [[nodiscard]] size_t size() const noexcept { return matrix.unique_sequences.size(); }

            /**
             * Find the unique sequence matching supplied operator string.
             * @param seq The sequence to match
             * @return Pointer to unique sequence element if matched, nullptr otherwise.
             */
            [[nodiscard]] const UniqueSequence * where(const OperatorSequence& seq) const noexcept;

            /**
             * Find symbol expression matching supplied operator sequence.
             * @param seq The sequence to match
             * @return The SymbolExpression matching the sequence, or zero if not found.
             */
            [[nodiscard]] SymbolExpression to_symbol(const OperatorSequence& seq) const noexcept;


            [[nodiscard]] const UniqueSequence& operator[](size_t index) const noexcept {
                assert(index < matrix.unique_sequences.size());
                return matrix.unique_sequences[index];
            }
        };


    private:
        const Context& context;
        const size_t hierarchy_level;
        size_t matrix_dimension;

        std::unique_ptr<SquareMatrix<OperatorSequence>> op_seq_matrix;
        std::unique_ptr<SquareMatrix<SymbolExpression>> sym_exp_matrix;

        std::vector<UniqueSequence> unique_sequences{};

        /** Maps hash to unique symbol */
        std::map<size_t, size_t> fwd_hash_table{};

        /** Maps hash to unique symbol's Hermitian conjugate */
        std::map<size_t, size_t> conj_hash_table{};

    public:
        /**
         * Range over unique sequences.
         */
        UniqueSequenceRange UniqueSequences;

        /**
         * View of symbolic matrix
         */
        SymbolMatrixView SymbolMatrix;

    public:
        NPAMatrix(const Context& the_context, size_t level);

        NPAMatrix(const NPAMatrix&) = delete;

        NPAMatrix(NPAMatrix&& src) noexcept :
            context{src.context}, hierarchy_level{src.hierarchy_level}, matrix_dimension{src.matrix_dimension},
            op_seq_matrix{std::move(src.op_seq_matrix)},
            sym_exp_matrix{std::move(src.sym_exp_matrix)},
            unique_sequences{std::move(src.unique_sequences)},
            UniqueSequences{*this}, SymbolMatrix{*this} { }

        /**
         * @return The number of rows in the matrix. Matrix is square, so also the number of columns.
         */
        [[nodiscard]] size_t dimension() const noexcept {
            return matrix_dimension;
        }

        /**
         * @return The number of rows and columns in the matrix. Matrix is square, so first and second are identical.
         */
        [[nodiscard]] std::pair<size_t, size_t> dimensions() const noexcept {
            return std::make_pair(matrix_dimension, matrix_dimension);
        }

        /**
         * Return a view (std::span<OperatorSequence>) to the supplied row of the NPA matrix. Since std::span also
         * provides an operator[], it is possible to index using "myNPAMatrix[row][col]" notation.
         * @param row The index of the row to return.
         * @return A std::span<OperatorSequence> of the requested row.
         */
        constexpr auto operator[](size_t row) const noexcept {
            return (*this->op_seq_matrix)[row];
        }

    private:
        /**
         * Find ID, and conjugation status, of element in unique_sequences corresponding to hash.
         * ID return will be numeric-limit max of size_t if no element found.
         * @param hash The hash to look up
         * @return Pair: First gives the element in unique_sequences, second is true if hash corresponds to conjugate.
         */
        [[nodiscard]] std::pair<size_t, bool> hashToElement(size_t hash) const noexcept;

        void identifyUniqueSequences(const std::vector<size_t> &hashes);

        std::unique_ptr<SquareMatrix<SymbolExpression>> buildSymbolMatrix(const std::vector<size_t> &hashes);
    };

}