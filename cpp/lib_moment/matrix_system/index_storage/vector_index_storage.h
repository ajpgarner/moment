/**
 * vector_index_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"
#include "matrix_system/matrix_indices.h"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <vector>

namespace Moment {

    /**
     * Stores matrix indices as a vector.
     * Useful when index is guaranteed to be a small number (e.g. NPA hierarchy level), providing constant time access.
     */
    template <std::convertible_to<size_t> index_t>
    class VectorIndexStorage {
    public:
        using Index = index_t;

        constexpr VectorIndexStorage() = default;

    private:
        std::vector<ptrdiff_t> the_indices;

    public:
        [[nodiscard]] constexpr ptrdiff_t find(const Index index) const noexcept {
            if (index >= the_indices.size()) {
                return -1;
            }
            return this->the_indices[static_cast<size_t>(index)]; // Could be -1 if not created.
        }

        [[nodiscard]] constexpr bool contains(const Index index) const noexcept {
            if (index >= the_indices.size()) {
                return false;
            }
            return this->the_indices[static_cast<size_t>(index)] != -1;
        }

        [[nodiscard]] constexpr auto insert(const Index index, ptrdiff_t offset)  {
            const size_t existing_count = the_indices.size();
            if (index >= existing_count) {
                std::fill_n(std::back_inserter(the_indices), (static_cast<size_t>(index) - existing_count), -1);
                the_indices.emplace_back(offset);
                return std::make_pair(offset, true);
            }
            if (the_indices[index] != -1) {
                return std::make_pair(the_indices[static_cast<size_t>(index)], false);
            }
            the_indices[index] = offset;
            return std::make_pair(offset, true);
        }

        [[nodiscard]] constexpr ptrdiff_t highest() const noexcept {
            return static_cast<ptrdiff_t>(this->the_indices.size()) - 1;
        }
    };


    static_assert(stores_indices<VectorIndexStorage<int>, int>);

    /**
     * Alias for matrix indices backed by std::vector, with size_t index
     */
    template<typename matrix_t, std::convertible_to<size_t> index_t,
             makes_matrices<matrix_t, index_t> factory_t, typename matrix_system_t>
    using VectorMatrixIndices = MatrixIndices<matrix_t, index_t, VectorIndexStorage<index_t>, factory_t, matrix_system_t>;

}