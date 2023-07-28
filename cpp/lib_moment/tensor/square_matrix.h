/**
 * square_matrix.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "tensor.h"

#include <cassert>

#include <algorithm>
#include <array>
#include <span>
#include <type_traits>
#include <vector>

namespace Moment {

    /**
     * Lightweight row-major square matrix of element_t.
     * @tparam element_t The elements of the matrix
     * @tparam storage_t The underlying storage class for the matrix data (e.g. vector, array).
     */
    template<class element_t, class storage_t = std::vector<element_t>>
            requires std::random_access_iterator<typename storage_t::const_iterator>
    class SquareMatrix : public Tensor<size_t, std::array<size_t, 2>, std::span<const size_t, 2>, true> {
    public:
        using TensorType = Tensor<size_t, std::array<size_t, 2>, std::span<const size_t, 2>, true>;

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

        /** Object for iterating over triangle of matrix */
        template<bool upper, bool inclusive, bool is_const>
        class TriangularIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = element_t;
            using reference = typename std::conditional<is_const, const element_t&, element_t&>::type;
            using matrix_ptr = typename std::conditional<is_const, const SquareMatrix*, SquareMatrix*>::type;
            using matrix_ref = typename std::conditional<is_const, const SquareMatrix&, SquareMatrix&>::type;


            /** For constructing end iterator */
            struct end_tag{};

        private:
            matrix_ptr matrix;
            typename SquareMatrix::Index index;
            size_t offset;

        public:
            explicit constexpr TriangularIterator(matrix_ref theMatrix)
                : matrix{&theMatrix},
                  index{inclusive ? 0 : (upper ? 0 : 1),
                        inclusive ? 0 : (upper ? 1 : 0)},
                  offset{inclusive ? 0 : (upper ? matrix->dimension : std::min<size_t>(1, matrix->ElementCount))} { }

            constexpr TriangularIterator(matrix_ref theMatrix, const end_tag& /**/)
                : matrix{&theMatrix},
                  index{upper ? 0 : theMatrix.dimension, theMatrix.dimension},
                  offset{(upper || !inclusive) ? theMatrix.ElementCount
                                               : theMatrix.ElementCount + theMatrix.dimension} { }

            [[nodiscard]] constexpr IndexView Index() const noexcept {
                return index;
            }

            [[nodiscard]] constexpr size_t Offset() const noexcept {
                return offset;
            }

            inline reference operator*() noexcept {
                return this->matrix->data[this->offset];
            }

            constexpr TriangularIterator& operator++() {
                ++offset;
                if constexpr(upper) { // UPPER triangle
                    if constexpr(inclusive) {
                        ++index[0]; // inc row index.
                        if (index[0] > index[1]) { // row > col
                            index[0] = 0; // back to top row
                            offset += (this->matrix->dimension - index[1] - 1); // skip remainder of column

                            ++index[1]; // next col
                        }
                    } else {
                        ++index[0]; // inc row index.
                        if (index[0] >= index[1]) { // row >= col
                            index[0] = 0; // back to top row
                            offset += (this->matrix->dimension - index[1]); // skip remainder of column
                            ++index[1]; // next col
                        }
                    }
                } else { // LOWER triangle
                    if constexpr(inclusive) {
                        ++index[0]; // inc row index.
                        if (index[0] >= this->matrix->dimension) { // row > matrix size
                            ++index[1]; // next col
                            index[0] = index[1]; // back to diagonal (of next col)
                            offset += index[1]; // skip first col elements of next col
                        }
                    } else {
                        ++index[0]; // inc row index.
                        if (index[0] >= this->matrix->dimension) { // row > matrix size
                            ++index[1]; // next col
                            index[0] = index[1]+1; // back to first off-diagonal (of next col)
                            offset += index[0]; // skip first col elements of next col
                        }
                    }
                }
                return *this;
            }

            [[nodiscard]] constexpr TriangularIterator operator++(int) & {
                TriangularIterator copy{*this};
                ++(*this);
                return copy;
            }

            [[nodiscard]] constexpr bool diagonal() const noexcept {
                if constexpr(inclusive) {
                    return this->index[0] == this->index[1];
                } else {
                    return false;
                }
            }

            template<bool other_upper, bool other_inclusive, bool other_is_const>
            [[nodiscard]] constexpr bool
            operator==(const TriangularIterator<other_upper, other_inclusive, other_is_const>& other) const noexcept {
                return this->offset == other.offset;
            }

            template<bool other_upper, bool other_inclusive, bool other_is_const>
            [[nodiscard]] constexpr bool
            operator!=(const TriangularIterator<other_upper, other_inclusive, other_is_const>& other) const noexcept {
                return this->offset != other.offset;
            }

        };

        /**
         * Range over matrix triangle
         */
        template<bool upper, bool inclusive, bool is_const>
        class TriangularView {
        public:
            using MatrixRef = typename std::conditional<is_const, const SquareMatrix&, SquareMatrix&>::type;
            MatrixRef matrix;

        public:
            explicit TriangularView(MatrixRef matrix) : matrix{matrix} { }

            /**
             * Read-write iterator to triangle if view is not const; otherwise read-only iterator.
             */
            [[nodiscard]] inline auto begin() noexcept {
                return TriangularIterator<upper, inclusive, is_const>{this->matrix};
            }

            /**
             * Read-write iterator to end of triangle if view is not const; otherwise read-only iterator.
             */
            [[nodiscard]] inline auto end() noexcept {
                return TriangularIterator<upper, inclusive, is_const>{this->matrix,
                          typename TriangularIterator<upper, inclusive, is_const>::end_tag{}};
            }

            /**
             * Read-only iterator to triangle.
             */
            [[nodiscard]] inline auto begin() const noexcept {
                return TriangularIterator<upper, inclusive, true>{this->matrix};
            }

            /**
             * Read-only iterator end to triangle.
             */
            [[nodiscard]] inline auto end() const noexcept {
                return TriangularIterator<upper, inclusive, true>{this->matrix,
                          typename TriangularIterator<upper, inclusive, true>::end_tag{}};
            }

            /**
             * Read-only iterator end to triangle.
             */
            [[nodiscard]] inline auto cbegin() const noexcept {
                return TriangularIterator<upper, inclusive, true>{this->matrix};
            }

            /**
              * Read-only iterator end to triangle.
              */
            [[nodiscard]] inline auto cend() const noexcept {
                return TriangularIterator<upper, inclusive, true>{this->matrix,
                      typename TriangularIterator<upper, inclusive, true>::end_tag{}};
            }

        };

        /**
         * Range over matrix upper triangle including diagonal.
         */
        using UpperTriangularView = TriangularView<true, true, false>;

        /**
         * Range over matrix upper triangle including diagonal.
         */
        using UpperTriangularConstView = TriangularView<true, true, true>;

        /**
         * Range over matrix upper triangle excluding diagonal.
         */
        using ExclusiveUpperTriangularView = TriangularView<true, false, false>;

        /**
         * Range over matrix upper triangle excluding diagonal.
         */
        using ExclusiveUpperTriangularConstView = TriangularView<true, false, true>;

        /**
         * Range over matrix lower triangle including diagonal.
         */
        using LowerTriangularView = TriangularView<false, true, false>;

        /**
         * Range over matrix lower triangle including diagonal.
         */
        using LowerTriangularConstView = TriangularView<false, true, true>;

        /**
         * Range over matrix lower triangle excluding diagonal.
         */
        using ExclusiveLowerTriangularView = TriangularView<false, false, false>;

        /**
         * Range over matrix lower triangle excluding diagonal.
         */
        using ExclusiveLowerTriangularConstView = TriangularView<false, false, true>;


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
        SquareMatrix() :  TensorType{std::array<size_t, 2>{0, 0}},
        dimension{0}, data{}, Transpose{*this} { }

        /** Move-construct square matrix */
        constexpr SquareMatrix(SquareMatrix&& rhs) noexcept
            : Tensor(static_cast<Tensor&&>(rhs)),
                dimension{rhs.dimension}, data{std::move(rhs.data)}, Transpose{*this} { }

        /**
         * Construct a square matrix from supplied data.
         * @param dimension The number of columns/rows in the square matrix
         * @param data Row-major data for matrix. Must contain dimension*dimension elements.
         */
        constexpr SquareMatrix(size_t dimension, storage_t&& data)
            : TensorType{std::array<size_t, 2>{dimension, dimension}},
              dimension{dimension}, data{std::move(data)}, Transpose{*this} {
            assert(this->data.size() == (this->dimension*this->dimension));
            assert(this->data.size() == this->ElementCount);
        }


        /**
         * Gets range over matrix upper triangle including diagonal.
         */
         UpperTriangularView UpperTriangle()  noexcept {
             return UpperTriangularView{*this};
         }

        /**
         * Gets range over matrix upper triangle including diagonal.
         */
         UpperTriangularConstView UpperTriangle() const noexcept {
             return UpperTriangularConstView{*this};
         }

        /**
         * Gets range over matrix upper triangle excluding diagonal.
         */
        ExclusiveUpperTriangularView ExclusiveUpperTriangle() noexcept {
            return ExclusiveUpperTriangularView{*this};
        };

        /**
         * Gets range over matrix upper triangle excluding diagonal.
         */
        ExclusiveUpperTriangularConstView ExclusiveUpperTriangle() const noexcept {
            return ExclusiveUpperTriangularConstView{*this};
        };

        /**
         * Gets range over matrix lower triangle including diagonal.
         */
        LowerTriangularView LowerTriangle() noexcept {
            return LowerTriangularView{*this};
        };

        /**
         * Gets range over matrix lower triangle including diagonal.
         */
        LowerTriangularConstView LowerTriangle() const noexcept {
            return LowerTriangularConstView{*this};
        };

        /**
         * Gets range over matrix lower triangle excluding diagonal.
         */
        ExclusiveLowerTriangularView ExclusiveLowerTriangle() noexcept  {
            return ExclusiveLowerTriangularView{*this};
        };

        /**
         * Gets range over matrix lower triangle excluding diagonal.
         */
        ExclusiveLowerTriangularConstView ExclusiveLowerTriangle() const noexcept  {
            return ExclusiveLowerTriangularConstView{*this};
        };


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
         * Get element by index.
         */
        constexpr inline const element_t& operator()(size_t row, size_t col) const noexcept(!debug_mode) {
            return this->operator()(Index{row, col});
        }

        /**
         * Get element by offset.
         */
        constexpr element_t& operator[](IndexElement offset) noexcept(!debug_mode) {
            if constexpr (debug_mode) {
                this->validate_offset(offset);
            }
            return this->data[offset];
        }

        /**
         * Get element by offset.
         */
        constexpr const element_t& operator[](IndexElement offset) const noexcept(!debug_mode) {
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