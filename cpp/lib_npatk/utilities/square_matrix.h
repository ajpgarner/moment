/**
 * square_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include <cassert>

#include <vector>
#include <span>

namespace NPATK {

    /**
     * Lightweight row-major square matrix of element_t.
     * @tparam element_t The elements of the matrix
     * @tparam storage_t The underlying storage class for the matrix data (e.g. vector, array).
     */
    template<class element_t, class storage_t = std::vector<element_t>>
            requires std::random_access_iterator<typename storage_t::const_iterator>
    class SquareMatrix {
    public:
        using iterator = typename storage_t::iterator;
        using const_iterator = typename storage_t::const_iterator;

    public:
        class ColumnMajorView {
        public:
            class TransposeIterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = element_t;
                using cref_type = const element_t &;
                using cptr_type = const element_t *;

            private:
                const SquareMatrix * theMatrix;
                size_t row = 0;
                size_t col = 0;
            public:
                /** Begin iterator */
                explicit constexpr TransposeIterator(const SquareMatrix& matrix) : theMatrix(&matrix) { }

                /** End iterator */
                constexpr TransposeIterator(const SquareMatrix& matrix, bool)
                    : theMatrix(&matrix), row{matrix.dimension}, col{matrix.dimension} { }

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

                constexpr TransposeIterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                [[nodiscard]] constexpr bool operator==(const TransposeIterator& rhs) const noexcept {
                    assert(this->theMatrix == rhs.theMatrix);
                    return (this->row == rhs.row) && (this->col == rhs.col);
                }

                [[nodiscard]] constexpr bool operator!=(const TransposeIterator& rhs) const noexcept {
                    return !this->operator==(rhs);
                }

                [[nodiscard]] cref_type operator*() const noexcept {
                    return theMatrix->data[(row* theMatrix->dimension) + col];
                }

                [[nodiscard]] cptr_type operator->() const noexcept {
                    return &(theMatrix->data[(row* theMatrix->dimension) + col]);
                }
            };

            static_assert(std::input_iterator<TransposeIterator>);
        private:

            const SquareMatrix& squareMatrix;
        public:
            explicit ColumnMajorView(const SquareMatrix& sm) : squareMatrix{sm} { }

            auto begin() const { return TransposeIterator{squareMatrix}; }
            auto end() const { return TransposeIterator{squareMatrix, true}; }

        };


    public:
        const size_t dimension;

    private:
        storage_t data;

    public:
        ColumnMajorView ColumnMajor;

    public:
        SquareMatrix() : dimension{0}, data{}, ColumnMajor{*this} { }

        constexpr SquareMatrix(SquareMatrix&& rhs) noexcept
            : dimension{rhs.dimension}, data{std::move(rhs.data)}, ColumnMajor{*this} { }

        constexpr SquareMatrix(size_t dimension, storage_t&& data)
            : dimension{dimension}, data{std::move(data)}, ColumnMajor{*this} {
            assert(this->data.size() == (this->dimension*this->dimension));
        }

        constexpr std::span<element_t> operator[](size_t row) noexcept {
            assert(row < this->dimension);
            auto iter_row_start = this->data.begin() + static_cast<ptrdiff_t>(row * this->dimension);
            return {iter_row_start.operator->(), this->dimension};
        }

        constexpr std::span<const element_t> operator[](size_t row) const noexcept {
            assert(row < this->dimension);
            auto iter_row_start = this->data.cbegin() + static_cast<ptrdiff_t>(row * this->dimension);
            return {iter_row_start.operator->(), this->dimension};
        }

        constexpr const_iterator begin() const noexcept { return this->data.cbegin(); }

        constexpr iterator begin() noexcept { return this->data.begin(); }

        constexpr const_iterator end() const noexcept { return this->data.cend(); }

        constexpr iterator end() noexcept { return this->data.end(); }

    };
}