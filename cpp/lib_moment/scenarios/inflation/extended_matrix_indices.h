/**
 * extended_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_indices.h"

#include "utilities/index_tree.h"
#include "utilities/maintains_mutex.h"
#include "utilities/set_to_vector.h"

#include "integer_types.h"

#include <set>
#include <vector>

namespace Moment {
    class MatrixSystem;
};

namespace Moment::Inflation {

    class ExtendedMatrix;
    class InflationMatrixSystem;

    struct ExtendedMatrixIndex {
    public:
        size_t moment_matrix_level;
    private:
        std::optional<std::vector<symbol_name_t>> internal_extension_list;
    public:
        std::span<const symbol_name_t> extension_list;

    public:
        ExtendedMatrixIndex(size_t mm_level, std::span<const symbol_name_t> list_span)
            : moment_matrix_level{mm_level}, extension_list(list_span) { }

        ExtendedMatrixIndex(size_t mm_level, std::vector<symbol_name_t>&& list)
                : moment_matrix_level{mm_level}, internal_extension_list(std::move(list)),
                  extension_list(internal_extension_list.value()) { }

        ExtendedMatrixIndex(size_t mm_level, const std::set<symbol_name_t>& list_as_set)
            : ExtendedMatrixIndex{mm_level, set_to_vector(list_as_set)} { }

        [[nodiscard]] constexpr bool stores_list() const noexcept {
            return internal_extension_list.has_value();
        }
    };

    class ExtendedMatrixIndexStorage final {
    public:
        using Index = ExtendedMatrixIndex;

    private:
        IndexTree<symbol_name_t, ptrdiff_t> extension_indices;

    public:
        [[nodiscard]] ptrdiff_t find(const Index& index) const noexcept;

        [[nodiscard]] bool contains(const Index& index) const noexcept;

        [[nodiscard]] std::pair<ptrdiff_t, bool> insert(const Index& index, ptrdiff_t offset);
    };
    static_assert(stores_indices<ExtendedMatrixIndexStorage, ExtendedMatrixIndex>);


    class ExtendedMatrixFactory final {
    private:
        InflationMatrixSystem& system;
    public:
        using Index = ExtendedMatrixIndex;

        explicit ExtendedMatrixFactory(MatrixSystem& system);

        explicit ExtendedMatrixFactory(InflationMatrixSystem& system) noexcept : system{system} { }

        [[nodiscard]] std::pair<ptrdiff_t, ExtendedMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const Index& index, ExtendedMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;

        MaintainsMutex::WriteLock get_write_lock();
    };

    static_assert(makes_matrices<ExtendedMatrixFactory, ExtendedMatrix, ExtendedMatrixIndex>);

    using ExtendedMatrixIndices = MatrixIndices<ExtendedMatrix, ExtendedMatrixIndex,
                                                ExtendedMatrixIndexStorage, ExtendedMatrixFactory,
                                                InflationMatrixSystem>;


}