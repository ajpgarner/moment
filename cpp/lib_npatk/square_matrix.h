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
     * @tparam element_t
     */
    template<class element_t, class storage_t = std::vector<element_t>>
    class SquareMatrix {
    private:
        storage_t data;
    public:
        const size_t dimension;

    public:
        constexpr SquareMatrix() : dimension{0}, data{} { }

        explicit SquareMatrix(size_t dimension) : dimension{dimension} { }

        constexpr SquareMatrix(SquareMatrix&& rhs) noexcept : dimension{rhs.dimension}, data{std::move(rhs.data)} { }

        constexpr SquareMatrix(size_t dimension, storage_t&& data) : dimension{dimension}, data{std::move(data)} {
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

        constexpr auto begin() const noexcept { return this->data.cbegin(); }

        constexpr auto begin() noexcept { return this->data.begin(); }

        constexpr auto end() const noexcept { return this->data.cend(); }

        constexpr auto end() noexcept { return this->data.end(); }

    };
}