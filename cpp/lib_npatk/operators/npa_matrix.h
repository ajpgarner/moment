/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"
#include "symbolic/symbol.h"

#include <cassert>
#include <memory>
#include <span>
#include <map>

namespace NPATK {
    class Context;


    class NPAMatrix {
    private:
        class UniqueSequence {
            size_t id = -1;
            OperatorSequence opSeq;
            std::optional<OperatorSequence> conjSeq{};
            size_t fwd_hash = 0;
            size_t conj_hash = 0;
            bool hermitian = false;

            UniqueSequence(OperatorSequence sequence, size_t hash) :
                    opSeq{std::move(sequence)}, fwd_hash{hash},
                    conjSeq{}, conj_hash{hash},
                    hermitian{true} { }

            UniqueSequence(OperatorSequence sequence, size_t hash,
                           OperatorSequence conjSequence, size_t conjHash) :
                    opSeq{std::move(sequence)}, fwd_hash{hash},
                    conjSeq{std::move(conjSequence)}, conj_hash{conjHash},
                    hermitian{false} { }

        public:
            [[nodiscard]] constexpr const OperatorSequence& sequence() const noexcept { return this->opSeq; }
            [[nodiscard]] constexpr const OperatorSequence& sequence_conj() const noexcept {
                return this->hermitian ? opSeq : this->conjSeq.value();
            }
            [[nodiscard]] constexpr bool is_hermitian() const noexcept { return this->hermitian; }

            friend class NPAMatrix;
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
            [[nodiscard]] const UniqueSequence& operator[](size_t index) const noexcept {
                assert(index < matrix.unique_sequences.size());
                return matrix.unique_sequences[index];
            }
        };


    private:
        const Context& context;
        const size_t hierarchy_level;
        size_t matrix_dimension;
        std::vector<OperatorSequence> matrix_data{};

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

    public:
        NPAMatrix(const Context& the_context, size_t level);

        /**
         * @return The number of rows in the matrix. Matrix is square, so also the number of columns.
         */
        [[nodiscard]] size_t dimension() const noexcept {
            return matrix_dimension;
        }

        /**
         * @return The number of rows and columns in the matrix. Matrix is square, so first and second are identical.
         */
        [[nodiscard]] auto dimensions() const noexcept {
            return std::make_pair(matrix_dimension, matrix_dimension);
        }


        /**
         * Find the unique sequence matching supplied operator string.
         * @param seq The sequence to match
         * @return Pointer to unique sequence element if matched, nullptr otherwise.
         */
        [[nodiscard]] const UniqueSequence * where(const OperatorSequence& seq) const noexcept;


        /**
         * Return a view (std::span<OperatorSequence>) to the supplied row of the NPA matrix. Since std::span also
         * provides an operator[], it is possible to index using "myNPAMatrix[row][col]" notation.
         * @param row The index of the row to return.
         * @return A std::span<OperatorSequence> of the requested row.
         */
        auto operator[](size_t row) const noexcept {
            assert(row < this->matrix_dimension);
            auto iter_row_start = this->matrix_data.begin() + static_cast<ptrdiff_t>(row * this->matrix_dimension);
            return std::span<const OperatorSequence>(iter_row_start.operator->(), this->matrix_dimension);
        }
    };

}