/**
 * square_matrix.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include <cassert>

#include <array>
#include <vector>
#include <span>

#include "integer_types.h"
#include "multi_dimensional_object.h"

namespace Moment {

    /**
     * Lightweight row-major square matrix of element_t.
     * @tparam element_t The elements of the matrix
     * @tparam storage_t The underlying storage class for the matrix data (e.g. vector, array).
     */
    template<class element_t, class storage_t = std::vector<element_t>>
            requires std::random_access_iterator<typename storage_t::const_iterator>
    class SquareMatrix
            : public MultiDimensionalObject<size_t, std::array<size_t, 2>, std::span<const size_t>, true>
    {
    public:
        using iterator = typename storage_t::iterator;
        using const_iterator = typename storage_t::const_iterator;

    public:

        /** Object for iterating over matrix data in transposed order. */
        const class TransposeView {
        public:
            class TransposeIterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = element_t;
                using cref_type = const element_t &;
                using cptr_type = const element_t *;
                using reference = const element_t &;

            private:
                const SquareMatrix * theMatrix;
                size_t row = 0;
                size_t col = 0;

            public:
                /** Begin iterator over transposed matrix. */
                explicit constexpr TransposeIterator(const SquareMatrix& matrix) : theMatrix(&matrix) { }

                /** End iterator over transposed matrix. */
                constexpr TransposeIterator(const SquareMatrix& matrix, bool)
                    : theMatrix(&matrix), row{matrix.dimension}, col{matrix.dimension} { }

                /** Increment iterator */
                constexpr TransposeIterator& operator++() {
                    ++row;
                    if (row >= theMatrix->dimension) {
                        ++col;
                        if (col >= theMatrix->dimension) {
                            return *this; // So that at end, index is (dim, dim)
                        }
                        row = 0;
                    }
                    return *this;
                }

                /** Increment iterator, and return copy of iterator pre-increment. */
                constexpr TransposeIterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                /** Compare iterators for equality */
                [[nodiscard]] constexpr bool operator==(const TransposeIterator& rhs) const noexcept {
                    assert(this->theMatrix == rhs.theMatrix);
                    return (this->row == rhs.row) && (this->col == rhs.col);
                }

                /** Compare iterators for inequality */
                [[nodiscard]] constexpr bool operator!=(const TransposeIterator& rhs) const noexcept {
                    return !this->operator==(rhs);
                }

                /** Return matrix element reference pointed to by iterator */
                [[nodiscard]] cref_type operator*() const noexcept {
                    return theMatrix->data[(row* theMatrix->dimension) + col];
                }

                /** Return pointer to matrix element pointed to by iterator */
                [[nodiscard]] cptr_type operator->() const noexcept {
                    return &(theMatrix->data[(row* theMatrix->dimension) + col]);
                }
            };

            static_assert(std::input_iterator<TransposeIterator>);
        private:
            const SquareMatrix& squareMatrix;

        public:
            /** Construct view for data, in column-major order. */
            explicit TransposeView(const SquareMatrix& sm) : squareMatrix{sm} { }

            /** Get column-major iterator start. */
            auto begin() const { return TransposeIterator{squareMatrix}; }

            /** Get column-major iterator end. */
            auto end() const { return TransposeIterator{squareMatrix, true}; }

            /** Get transposed element */
            inline const element_t& operator()(IndexView index) const noexcept(!debug_mode) {
                return this->squareMatrix(Index{index[1], index[0]});
            }

        } Transpose;

        /** Type alias for matrix data array. */
        using StorageType = storage_t;

    public:
        /** The number of columns/rows in the square matrix. */
        const IndexElement dimension;

    private:
        /** Matrix data */
        StorageType data;

    public:

    public:
        /** Construct empty, 0 by 0, matrix */
        SquareMatrix() : MultiDimensionalObject{std::array<size_t, 2>{0, 0}},
        dimension{0}, data{}, Transpose{*this} { }

        /** Move-construct square matrix */
        constexpr SquareMatrix(SquareMatrix&& rhs) noexcept
            : MultiDimensionalObject(static_cast<MultiDimensionalObject&&>(rhs)),
                dimension{rhs.dimension}, data{std::move(rhs.data)}, Transpose{*this} { }

        /**
         * Construct a square matrix from supplied data.
         * @param dimension The number of columns/rows in the square matrix
         * @param data Row-major data for matrix. Must contain dimension*dimension elements.
         */
        constexpr SquareMatrix(size_t dimension, storage_t&& data)
            : MultiDimensionalObject{std::array<size_t, 2>{dimension, dimension}},
              dimension{dimension}, data{std::move(data)}, Transpose{*this} {
            assert(this->data.size() == (this->dimension*this->dimension));
        }

        /**
         * Read/write access a row of the square matrix.
         * @param row The row's index.
         * @return Span over elements in the row.
         */
        constexpr std::span<element_t> operator[](size_t row) noexcept {
            assert(row < this->dimension);
            auto iter_row_start = this->data.begin() + static_cast<ptrdiff_t>(row * this->dimension);
            return {iter_row_start.operator->(), this->dimension};
        }

        /**
         * Read access a row of the square matrix.
         * @param row The row's index.
         * @return Span over elements in the row.
         */
        constexpr std::span<const element_t> operator[](size_t row) const noexcept {
            assert(row < this->dimension);
            auto iter_row_start = this->data.cbegin() + static_cast<ptrdiff_t>(row * this->dimension);
            return {iter_row_start.operator->(), this->dimension};
        }

        /**
         * Get element by index.
         */
        constexpr element_t& operator()(IndexView index) noexcept(!debug_mode) {
            if constexpr (debug_mode) {
                this->validate_index(index);
            }
            auto offset = this->index_to_offset_no_checks(index);
            return this->data[offset];
        }

        /**
         * Get element by index.
         */
        constexpr const element_t& operator()(IndexView index) const noexcept(!debug_mode) {
            if constexpr (debug_mode) {
                this->validate_index(index);
            }
            auto offset = this->index_to_offset_no_checks(index);
            return this->data[offset];
        }

        /**
         * Get element by offset.
         */
        constexpr element_t& operator()(IndexElement offset) noexcept(!debug_mode) {
            if constexpr (debug_mode) {
                this->validate_offset(offset);
            }
            return this->data[offset];
        }

        /**
         * Get element by offset.
         */
        constexpr const element_t& operator()(IndexElement offset) const noexcept(!debug_mode) {
            if constexpr (debug_mode) {
                this->validate_offset(offset);
            }
            return this->data[offset];
        }

        /** Gets a column-major read-only iterator over matrix data. */
        constexpr const_iterator begin() const noexcept { return this->data.cbegin(); }

        /** Gets a column-major iterator over matrix data. */
        constexpr iterator begin() noexcept { return this->data.begin(); }

        /** Gets the end of the column-major read-only iterator over matrix data. */
        constexpr const_iterator end() const noexcept { return this->data.cend(); }

        /** Gets the end of the column-major iterator over matrix data. */
        constexpr iterator end() noexcept { return this->data.end(); }

        /** Gets raw data */
        const element_t * raw() const noexcept { return this->data.data(); }

        /**
         * Create new square matrix with this matrix as the principle submatrix.
         * @param padding The number of 'zeros' to pad with
         * @param zero Reference 'zero' value.
         * @return New square matrix of dimension "dimension + padding".
         */
        [[nodiscard]] SquareMatrix pad(size_t padding, const element_t zero) const {
            const size_t new_dimension = this->dimension + padding;
            storage_t new_data;

            for (size_t i = 0; i < this->dimension; ++i) {
                const auto old_data_iter = this->data.begin() + (i * this->dimension);
                const auto old_data_iter_end = this->data.begin() + ((i+1) * this->dimension);
                // Copy
                std::copy(old_data_iter, old_data_iter_end, std::back_inserter(new_data));
                // Pad
                new_data.insert(new_data.end(), padding, zero);
            }

            const size_t remaining_zeros = padding * new_dimension;
            // Pad
            new_data.insert(new_data.end(), remaining_zeros, zero);
            return SquareMatrix(new_dimension, std::move(new_data));
        }

    };
}