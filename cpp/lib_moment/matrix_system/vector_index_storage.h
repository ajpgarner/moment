/**
 * vector_index_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace Moment {

    /**
     * Stores matrix indices as a vector.
     * Useful when index is guaranteed to be a small number (e.g. NPA hierarchy level), providing constant time access.
     */
    class VectorIndexStorage {
    public:
        using Index = size_t;

        constexpr VectorIndexStorage() = default;

    private:
        std::vector<ptrdiff_t> the_indices;

    public:
        [[nodiscard]] constexpr ptrdiff_t find(const Index index) const noexcept {
            if (index >= the_indices.size()) {
                return -1;
            }
            return this->the_indices[index]; // Could be -1 if not created.
        }

        [[nodiscard]] constexpr bool contains(const Index index) const noexcept {
            if (index >= the_indices.size()) {
                return false;
            }
            return this->the_indices[index] != -1;
        }

        [[nodiscard]] constexpr auto insert(const Index index, ptrdiff_t offset)  {
            const size_t existing_count = the_indices.size();
            if (index >= existing_count) {
                std::fill_n(std::back_inserter(the_indices), (index - existing_count), -1);
                the_indices.emplace_back(offset);
                return std::make_pair(offset, true);
            }
            if (the_indices[index] != -1) {
                return std::make_pair(the_indices[index], false);
            }
            the_indices[index] = offset;
            return std::make_pair(offset, true);
        }

        [[nodiscard]] constexpr ptrdiff_t highest() const noexcept {
            return static_cast<ptrdiff_t>(this->the_indices.size()) - 1;
        }
    };
}