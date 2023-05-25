/**
 * triangular_index_iterator.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <concepts>
#include <numeric>
#include <vector>

#include "ipow.h"

namespace Moment {

    /**
     * Iterate over ordered sequences.
     * @tparam index_t The type of index
     * @tparam size_storage_t The storage container for the indices
     * @tparam allow_duplicates If true, allow strings with identical elements.
     */
    template<std::integral index_t, typename storage_t = typename std::vector<index_t>,
            bool allow_duplicates = false>
    class TriangularIndexIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = storage_t;
        using element_type = index_t;
        static constexpr bool WithDuplicates = allow_duplicates;

        const index_t maximum_index;
        const size_t word_length;
    private:

        size_t global_index = 0;
        storage_t indices;
        bool is_done = false;

    public:
        constexpr explicit TriangularIndexIterator(index_t max_index, size_t word_length, bool end = false)
            : maximum_index{max_index}, word_length(word_length), indices(word_length, 0), is_done{end} {
            // No iteration if no indices.
            if ((word_length == 0) || (max_index == 0)) {
                 this->is_done = true;
            }
            assert(indices.size() == word_length);

            // If done, set global index to be product of items
            if (this->is_done) {
                this->global_index = ipow(static_cast<size_t>(maximum_index), word_length);
            } else {
                // Set initial state
                if constexpr(!allow_duplicates) {
                    std::iota(this->indices.begin(), this->indices.end(), index_t{0});
                }
            }
        }

        constexpr TriangularIndexIterator& operator++() noexcept {
            size_t recurse_depth = word_length - 1;

            if constexpr (allow_duplicates) {
                bool recursing = true;
                while (recursing) {
                    ++this->indices[recurse_depth];
                    if (this->indices[recurse_depth] >= maximum_index) {
                        if (recurse_depth > 0) {
                            --recurse_depth;
                        } else {
                            // leave all at max index, indicating done state!
                            this->is_done = true;
                            recursing = false;
                        }
                    } else {
                        recursing = false;
                    }
                }

                // Correction vs skip...!
                if (!this->is_done) {
                    index_t val = this->indices[recurse_depth];
                    ++recurse_depth;
                    while (recurse_depth < this->word_length) {
                        this->indices[recurse_depth++] = val;
                    }
                }

            } else {
                bool recursing = true;
                while (recursing) {
                    ++this->indices[recurse_depth];
                    const index_t effective_maximum = 1 + maximum_index + recurse_depth - this->word_length;
                    if (this->indices[recurse_depth] >= effective_maximum) {
                        if (recurse_depth > 0) {
                            --recurse_depth;
                        } else {
                            // leave all at max index, indicating done state!
                            this->is_done = true;
                            recursing = false;
                        }
                    } else {
                        recursing = false;
                    }
                }
                // Correction vs skip...!
                if (!this->is_done) {
                    index_t val = this->indices[recurse_depth] + 1;
                    ++recurse_depth;
                    while (recurse_depth < this->word_length) {
                        this->indices[recurse_depth++] = val++;
                    }
                }
            }

            ++this->global_index;

            return *this;
        }

        [[nodiscard]] constexpr TriangularIndexIterator operator++(int)& noexcept { // NOLINT(cert-dcl21-cpp)
            TriangularIndexIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] constexpr bool operator==(const TriangularIndexIterator& rhs) const noexcept {
            // Cannot be equal if one iter is done and the other not
            if (this->is_done != rhs.is_done) {
                return false;
            }

            // All "done" iterators are equivalent.
            if (this->is_done) {
                return true;
            }

            // Compare global index
            return this->global_index == rhs.global_index;
        }

        [[nodiscard]] constexpr bool operator!=(const TriangularIndexIterator& rhs) const noexcept {
            return !this->operator==(rhs);
        }

        constexpr const value_type& operator*() const noexcept {
            return this->indices;
        }

        [[nodiscard]] constexpr element_type operator[](size_t dim) const noexcept {
            assert(dim < this->word_length);
            return this->indices[dim];
        };

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

    template<std::integral index_t = size_t, typename storage_t = std::vector<index_t>>
    using DuplicateTriangularIndexIterator = TriangularIndexIterator<index_t, storage_t, true>;

    template<std::integral index_t = size_t, typename storage_t = std::vector<index_t>>
    using UniqueTriangularIndexIterator = TriangularIndexIterator<index_t, storage_t, false>;

}
