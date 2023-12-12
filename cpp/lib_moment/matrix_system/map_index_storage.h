/**
 * map_index_storage.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_indices.h"

#include <map>

namespace Moment {

    /**
     * Matrix index storage using std::map directly.
     */
    template<typename index_t>
    class MapIndexStorage {
    public:
        using Index = index_t;

    private:
        std::map<Index, ptrdiff_t> the_map;

    public:
        [[nodiscard]] inline ptrdiff_t find(const Index& index) const noexcept {
            auto where = the_map.find(index);
            return (where == the_map.cend()) ? -1 : where->second;
        }

        [[nodiscard]] inline bool contains(const Index& index) const noexcept {
            return the_map.contains(index);
        }

        [[nodiscard]] inline auto insert(const Index& index, ptrdiff_t offset)  {
            auto [where, did_insert] = the_map.emplace(index, offset);
            return std::make_pair(where->second, did_insert);
        }
    };
    static_assert(stores_indices<MapIndexStorage<int>, int>);

    /**
     * Alias for matrix indices backed by std::map.
     */
    template<typename matrix_t, typename index_t, makes_matrices<matrix_t, index_t> factory_t, typename matrix_system_t>
    using MappedMatrixIndices = MatrixIndices<matrix_t, index_t, MapIndexStorage<index_t>, factory_t, matrix_system_t>;
}