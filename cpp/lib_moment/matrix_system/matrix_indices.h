/**
 * matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"
#include "matrix_system_errors.h"

#include "utilities/multithreading.h"

#include <cassert>
#include <concepts>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace Moment {
    class Matrix;
    class MatrixSystem;

    /**
     * Concept: type that maps indices of type index_t to offsets of type ptrdiff_t.
     */
    template<typename index_storage_t, typename index_t>
    concept stores_indices = requires (index_storage_t& index_store, const index_t& index, const ptrdiff_t offset) {
        {index_store.find(index)} -> std::convertible_to<ptrdiff_t>;
        {index_store.contains(index)} -> std::convertible_to<bool>;
        {index_store.insert(index, offset)} -> std::convertible_to<std::pair<ptrdiff_t, bool>>;
    };

    /**
     * Concept: object that describes indices, and can create new matrices from them.
     * @tparam matrix_t
     * @tparam index_t
     * @tparam factory_t
     */
    template<typename factory_t, typename matrix_t, typename index_t>
    concept makes_matrices =
        std::is_same_v<typename factory_t::Index, index_t> &&
        requires (factory_t& factory, const factory_t& const_factory,
                  const index_t& index, matrix_t& matrix_ref, std::unique_lock<std::shared_mutex>& lock,
                  const Multithreading::MultiThreadPolicy& mt_policy) {
            {factory.get_write_lock()};
            {factory(lock, index, mt_policy)} -> std::convertible_to<std::pair<ptrdiff_t, matrix_t&>>;
            {factory.notify(index, matrix_ref)};
            {const_factory.not_found_msg(index)} -> std::convertible_to<std::string>;
        };

    /**
     * Maps a sub-set of matrices from MatrixSystem, with a custom index, to the storage location of said matrices.
     * @tparam matrix_t The matrix type.
     * @tparam index_t The index type.
     * @tparam index_storage_t Container for storing indices.
     * @tparam index_info_t Description for indices.
     */
    template<typename matrix_t,
            typename index_t,
            stores_indices<index_t> index_storage_t,
            makes_matrices<matrix_t, index_t> factory_t,
            typename matrix_system_t>
    class MatrixIndices {
    public:

        using MatrixSystemType = matrix_system_t;
        using Index = index_t;
        using IndexStorage = index_storage_t;
        using MatrixType = matrix_t;
        using FactoryType = factory_t;
        using MTPolicy = Multithreading::MultiThreadPolicy;
        friend MatrixSystemType;

    private:
        IndexStorage indices;
        matrix_system_t& system;
        FactoryType matrixFactory;

    public:
        template<typename... Args>
        explicit MatrixIndices(matrix_system_t& system, IndexStorage&& index, Args&&... args)
            : system{system}, indices{index}, matrixFactory{system, std::forward<Args>(args)...} { }

        template<typename... Args>
        explicit MatrixIndices(matrix_system_t& system, Args&&... args)
            : system{system}, matrixFactory{system, std::forward<Args>(args)...} { }

    public:
        [[nodiscard]] const IndexStorage& Indices() const noexcept {
            return indices;
        }

        /**
         * Create matrix with requested index, or retrieve if already existing.
         * @param index The description of the matrix.
         * @param mt_policy The multi-threaded policy for creation.
         * @return Offset of the matrix within the matrix system, and reference to the matrix.
         */
        [[nodiscard]] std::pair<size_t, matrix_t&>
        create(const index_t& index, const MTPolicy mt_policy = MTPolicy::Optional) {
            // Get write lock from factory.
            auto lock = matrixFactory.get_write_lock();
            return this->create(lock, index, mt_policy); //~ releases lock
        }

        /**
         * Create matrix with requested index, or retrieve if already existing.
         * @param index The description of the matrix.
         * @param mt_policy The multi-threaded policy for creation.
         * @return Offset of the matrix within the matrix system, and reference to the matrix.
         */
        [[nodiscard]] std::pair<size_t, matrix_t&>
        create(std::unique_lock<std::shared_mutex>& lock,
               const index_t& index, const MTPolicy mt_policy = MTPolicy::Optional) {
            // Must hold write lock
            assert(this->system.is_locked_write_lock(lock));

            // Does matrix supposedly already exist?
            auto existing = this->indices.find(index);
            if (existing >= 0) {
                try {
                    if constexpr (std::is_same_v<MatrixType, Moment::Matrix>) {
                        return std::pair<size_t, MatrixType&>{static_cast<size_t>(existing), this->system.get(existing)};
                    } else {
                        return std::pair<size_t, MatrixType&>{static_cast<size_t>(existing),
                                                              dynamic_cast<MatrixType &>(this->system.get(existing))};
                    }
                } catch (const errors::missing_component& mce) {
                    throw std::runtime_error{"Index for matrix was found, but matrix was invalid."};
                } catch (const std::bad_cast& bad_cast) {
                    throw std::runtime_error{"Index for matrix was found, but matrix was of invalid type."};
                }
            }

            // Otherwise, call factory to actually handle insertion into system.
            auto [matrix_offset, matrix_ref] = matrixFactory(lock, index, mt_policy);
            [[maybe_unused]] const auto [actual_offset, did_insertion] = this->indices.insert(index, matrix_offset);
            assert(actual_offset == matrix_offset);
            assert(did_insertion);
            matrixFactory.notify(index, matrix_ref);

            return std::pair<size_t, matrix_t&>{static_cast<size_t>(matrix_offset), matrix_ref};
        }

        /**
         * Retrieve matrix with requested index.
         * @param index The description of the matrix.
         * @return Offset of the matrix within the matrix system, and reference to the matrix.
         * @throws Moment::errors::missing_component if no matrix exists at index.
         */
        [[nodiscard]] const MatrixType& find(const index_t& index) const {
            auto where = this->indices.find(index);
            if (where < 0) {
                throw Moment::errors::missing_component(matrixFactory.not_found_msg(index));
            }

            if constexpr(std::is_same_v<MatrixType, Moment::Matrix>) {
                return this->system[where]; // throws if bad offset.
            } else {
                return dynamic_cast<MatrixType&>(this->system.matrices[where]); // throws if bad offset.
            }
        }

        [[nodiscard]] inline ptrdiff_t find_index(const index_t& index) const noexcept {
            return this->indices.find(index);
        }

        [[nodiscard]] inline bool contains(const index_t& index) const noexcept {
            return this->indices.contains(index);
        }


        [[nodiscard]] inline const MatrixType& operator()(const index_t& index) const {
            return this->find(index);
        }

        [[nodiscard]] inline MatrixType& operator()(const index_t& index,
                                                    const MTPolicy mt_policy = MTPolicy::Optional) {
            auto lock = matrixFactory.get_write_lock();
            auto [offset, matrix] = this->create(lock, index, mt_policy);
            return matrix; // ~releases lock
        }

    };

    /**
     * Matrix index storage using std::map directly.
     */
    template<typename index_t>
    class MatrixIndexViaStdMap {
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
    static_assert(stores_indices<MatrixIndexViaStdMap<int>, int>);

    /**
     * Alias for matrix indices backed by std::map.
     */
    template<std::derived_from<Moment::Matrix> matrix_t,
            typename index_t,
            makes_matrices<matrix_t, index_t> factory_t,
            typename matrix_system_t>
    using MappedMatrixIndices = MatrixIndices<matrix_t, index_t, MatrixIndexViaStdMap<index_t>,
                                              factory_t, matrix_system_t>;

}
