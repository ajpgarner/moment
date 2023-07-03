/**
 * multi_dimensional_index_iterator.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>
#include <cstddef>

#include <vector>

namespace Moment {

    template<bool reversed_indices = false, class size_storage_t = std::vector<size_t>>
    class MultiDimensionalOffsetIndexIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = std::vector<size_t>;

    private:
        size_t num_indices;

        size_t global_index = 0;

        size_storage_t min_vals;
        size_storage_t max_vals;
        size_storage_t indices;

        bool is_done = false;

    public:
        constexpr explicit MultiDimensionalOffsetIndexIterator(size_storage_t _min_vals, size_storage_t _max_vals)
                : num_indices{_min_vals.size()}, min_vals(std::move(_min_vals)), max_vals(std::move(_max_vals)),
                  indices(min_vals), is_done{false} {

            assert(this->min_vals.size() == this->max_vals.size());

            // No iteration if no indices.
            if (num_indices == 0) {
                this->is_done = true;
            }

            // No iteration if one index is empty.
            for (size_t n = 0; n < num_indices; ++n) {
                if (this->min_vals[n] == this->max_vals[n]) {
                    this->is_done = true;
                }
            }

            // If done, set global index to product of items
            if (this->is_done) {
                this->global_index = 1;
                for (auto max_val : this->max_vals) {
                    this->global_index *= max_val;
                }
            }
        }

        /** End iterator */
        constexpr explicit MultiDimensionalOffsetIndexIterator()
            : num_indices{0}, is_done{true} {

        }

        constexpr MultiDimensionalOffsetIndexIterator& operator++() noexcept {
            assert(!this->is_done);

            if constexpr (reversed_indices) {
                size_t recurse_depth = 0;
                bool recursing = true;
                while (recursing) {
                    ++this->indices[recurse_depth];
                    if (this->indices[recurse_depth] >= this->max_vals[recurse_depth]) {
                        this->indices[recurse_depth] = this->min_vals[recurse_depth];
                        if (recurse_depth < num_indices - 1) {
                            ++recurse_depth;
                        } else {
                            this->is_done = true;
                            recursing = false;
                        }
                    } else {
                        recursing = false;
                    }
                }
            } else {

                // forward indices
                size_t recurse_depth = num_indices - 1;
                bool recursing = true;
                while (recursing) {
                    ++this->indices[recurse_depth];
                    if (this->indices[recurse_depth] >= this->max_vals[recurse_depth]) {
                        this->indices[recurse_depth] = this->min_vals[recurse_depth];
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
            }
            ++this->global_index;

            return *this;
        }

        [[nodiscard]] constexpr MultiDimensionalOffsetIndexIterator operator++(int)& noexcept { // NOLINT(cert-dcl21-cpp)
            MultiDimensionalOffsetIndexIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] constexpr bool operator==(const MultiDimensionalOffsetIndexIterator& rhs) const noexcept {
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

        [[nodiscard]] constexpr bool operator!=(const MultiDimensionalOffsetIndexIterator& rhs) const noexcept {
            return !this->operator==(rhs);
        }

        constexpr const size_storage_t& operator*() const noexcept {
            return this->indices;
        }

        [[nodiscard]] constexpr size_t operator[](size_t dim) const noexcept {
            assert(dim < this->num_indices);
            return this->indices[dim];
        };

        [[nodiscard]] constexpr const size_storage_t& lower_limits() const noexcept {
            return this->min_vals;
        }

        [[nodiscard]] constexpr const size_storage_t& upper_limits() const noexcept {
            return this->max_vals;
        }

        /**
         * True, if iterator is not done.
         */
        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return !this->is_done;
        }

        /**
         * True, if iterator is done.
         */
        [[nodiscard]] constexpr bool operator!() const noexcept {
            return this->is_done;
        }

        [[nodiscard]] constexpr size_t global() const noexcept {
            return this->global_index;
        }

    };

}
