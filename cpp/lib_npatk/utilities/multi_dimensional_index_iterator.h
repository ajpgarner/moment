/**
 * multi_dimensional_index_iterator.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include <vector>
#include <cassert>

namespace NPATK {

    class MultiDimensionalIndexIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = std::vector<size_t>;

    private:
        size_t num_indices;
        std::vector<size_t> max_vals;
        std::vector<size_t> indices;
        bool is_done = false;

    public:
        explicit MultiDimensionalIndexIterator(std::vector<size_t> sizes, bool end = false)
            : num_indices(sizes.size()), max_vals(std::move(sizes)), indices(num_indices, 0), is_done{end} {
            // No iteration if no indices.
            if (num_indices == 0) {
                 this->is_done = true;
            }

            // No iteration if one index is empty.
            for (auto val : max_vals) {
                if (val == 0) {
                    this->is_done = true;
                }
            }
        }

        MultiDimensionalIndexIterator& operator++() noexcept {
            size_t recurse_depth = num_indices - 1;
            bool recursing = true;
            while (recursing) {
                ++this->indices[recurse_depth];
                if (this->indices[recurse_depth] >= this->max_vals[recurse_depth]) {
                    this->indices[recurse_depth] = 0;
                    if (recurse_depth > 0) {
                        --recurse_depth;
                    } else {
                        this->is_done = true;
                        recursing = false;
                    }
                } else {
                    // Party is not at end...
                    recursing = false;
                }
            }
            return *this;
        }

        [[nodiscard]] MultiDimensionalIndexIterator operator++(int)& noexcept {
            MultiDimensionalIndexIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] bool operator==(const MultiDimensionalIndexIterator& rhs) const noexcept {
            // Cannot be equal if one iter is done and the other not
            if (this->is_done != rhs.is_done) {
                return false;
            }

            // All "done" iterators are equivalent.
            if (this->is_done) {
                return true;
            }

            // Otherwise, compare indices
            assert(this->num_indices == rhs.num_indices);
            for (size_t i = 0; i < this->num_indices; ++i) {
                if (this->indices[i] != rhs.indices[i]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool operator!=(const MultiDimensionalIndexIterator& rhs) const noexcept {
            return !this->operator==(rhs);
        }

        const std::vector<size_t>& operator*() const noexcept {
            return this->indices;
        }

        size_t operator[](size_t dim) const noexcept {
            assert(dim < this->num_indices);
            return this->indices[dim];
        };

        [[nodiscard]] const std::vector<size_t>& limits() const noexcept {
            return this->max_vals;
        }

        [[nodiscard]] bool done() const noexcept {
            return this->is_done;
        }

    };

    static_assert(std::input_iterator<MultiDimensionalIndexIterator>);

    class  MultiDimensionalIndexRange {
    private:
        std::vector<size_t> max_vals;
    public:
        explicit MultiDimensionalIndexRange(std::vector<size_t> limits) : max_vals(std::move(limits)) { }

        MultiDimensionalIndexRange(std::initializer_list<size_t> limits) : max_vals{limits} { }

        [[nodiscard]] auto begin() const noexcept {
            return MultiDimensionalIndexIterator{max_vals};
        }

        [[nodiscard]] auto end() const noexcept {
            return MultiDimensionalIndexIterator{max_vals, true};
        }
    };

}