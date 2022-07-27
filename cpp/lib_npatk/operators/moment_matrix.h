/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "context.h"
#include "operator_sequence.h"
#include "symbolic/symbol_expression.h"
#include "symbolic/index_matrix_properties.h"
#include "utilities/square_matrix.h"

#include <cassert>
#include <atomic>
#include <map>
#include <memory>
#include <span>

namespace NPATK {

    class CollinsGisinForm;
    class ImplicitSymbols;

    class MomentMatrix {
    public:
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
            [[nodiscard]] constexpr symbol_name_t Id() const noexcept { return this->id; }
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

            friend class MomentMatrix;
        };

        class SymbolMatrixView {
        private:
            const MomentMatrix& matrix;
        public:
            explicit SymbolMatrixView(const MomentMatrix& theMatrix) : matrix{theMatrix} { };
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
            const MomentMatrix& matrix;
        public:
            explicit UniqueSequenceRange(const MomentMatrix& theMatrix) : matrix{theMatrix} { }
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
        /** Moment matrix takes partial ownership over context - so it is not destroyed before moment matrix. */
        std::shared_ptr<Context> contextPtr;


    public:
        /** Scenario */
        const Context& context;

        /** The level of moment matrix defined */
        const size_t hierarchy_level;

        /**
         * The maximum length operator sequence found in this moment matrix that corresponds to a probability.
         * Effectively: min(number of parties, 2 * hierarchy_level)
         */
        const size_t max_probability_length;

        /**
         * Range over unique sequences.
         */
        UniqueSequenceRange UniqueSequences;

        /**
         * View of symbolic matrix
         */
        SymbolMatrixView SymbolMatrix;

    private:
        size_t matrix_dimension;

        std::unique_ptr<SquareMatrix<OperatorSequence>> op_seq_matrix;

        std::unique_ptr<SquareMatrix<SymbolExpression>> sym_exp_matrix;

        std::vector<UniqueSequence> unique_sequences{};

        IndexMatrixProperties imp;

        /** Maps hash to unique symbol */
        std::map<size_t, size_t> fwd_hash_table{};

        /** Maps hash to unique symbol's Hermitian conjugate */
        std::map<size_t, size_t> conj_hash_table{};

        /** Map of measurement outcome symbols */
        std::unique_ptr<CollinsGisinForm> cgForm;

        /** Map of implied probabilities */
        std::unique_ptr<ImplicitSymbols> implicitSymbols;

        /** For thread-safe construction of cgForm */
        mutable std::atomic_flag cgFormExists;
        mutable std::atomic_flag cgFormConstructFlag;

        /** For thread-safe construction of implicit symbols */
        mutable std::atomic_flag impSymExists;
        mutable std::atomic_flag impSymConstructFlag;

    public:
        MomentMatrix(std::shared_ptr<Context> contextPtr, size_t level);

        MomentMatrix(const MomentMatrix&) = delete;

        MomentMatrix(MomentMatrix&& src) noexcept;

        /** Destructor */
        ~MomentMatrix();

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

        [[nodiscard]] constexpr size_t level() const noexcept { return this->hierarchy_level; }

        [[nodiscard]] const SquareMatrix<OperatorSequence>& SequenceMatrix() const noexcept {
            return *this->op_seq_matrix;
        }

        [[nodiscard]] const IndexMatrixProperties& BasisIndices() const noexcept { return this->imp; }

        [[nodiscard]] const CollinsGisinForm& CollinsGisin() const;

        [[nodiscard]] const ImplicitSymbols& ImplicitSymbolTable() const;

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