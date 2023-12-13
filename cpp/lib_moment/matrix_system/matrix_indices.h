/**
 * matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * Generic types, allows for access of a subset of MatrixSystem matrices via a subset-specific index, or invoke creation
 * of said matrices, parameterized by this index.
 *
 */

#pragma once

#include "integer_types.h"
#include "matrix_system_errors.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

#include <cassert>
#include <concepts>
#include <memory>
#include <string>
#include <utility>

namespace Moment {
    class SymbolicMatrix;
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
                  const index_t& index, ptrdiff_t offset, matrix_t& matrix_ref, MaintainsMutex::WriteLock& lock,
                  const MaintainsMutex::WriteLock& write_lock,
                  const Multithreading::MultiThreadPolicy& mt_policy) {
            {factory(lock, index, mt_policy)} -> std::convertible_to<std::pair<ptrdiff_t, matrix_t&>>;
            {factory.notify(write_lock, index, offset, matrix_ref)};
        };

    /**
     * Generic type, allows for access of a subset of MatrixSystem matrices via a subset-specific index.
     * @tparam matrix_t The matrix type.
     * @tparam index_t The index type.
     * @tparam index_storage_t Container for storing indices.
     * @tparam factory_t An object that can construct new versions of matrices from their index.
     * @tparam matrix_system_t The explicit matrix system type.
     */
    template<typename matrix_t,
            typename index_t,
            stores_indices<index_t> index_storage_t,
            makes_matrices<matrix_t, index_t> factory_t,
            typename matrix_system_t>
    class MatrixIndices final {
    public:

        using MatrixSystemType = matrix_system_t;
        using Index = index_t;
        using IndexStorage = index_storage_t;
        using MatrixType = matrix_t;
        using FactoryType = factory_t;
        using MTPolicy = Multithreading::MultiThreadPolicy;
        friend MatrixSystemType;

    private:
        matrix_system_t& system;
        IndexStorage indices;
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
            auto lock = this->system.get_write_lock();
            return this->create(lock, index, mt_policy);
            //~ releases lock
        }

        /**
         * Create matrix with requested index, or retrieve if already existing.
         * @param index The description of the matrix.
         * @param mt_policy The multi-threaded policy for creation.
         * @return Offset of the matrix within the matrix system, and reference to the matrix.
         */
        [[nodiscard]] std::pair<size_t, matrix_t&>
        create(const MaintainsMutex::WriteLock& lock, const index_t& index, const MTPolicy mt_policy = MTPolicy::Optional) {
            // Must hold write lock
            assert(this->system.is_locked_write_lock(lock));

            // Does matrix supposedly already exist? [NB: Even if we just checked, we must include this, as part of the
            //  double-checked locking paradigm, as the matrix might have been created in a race].
            auto existing = this->indices.find(index);
            if (existing >= 0) {
                try {
                    if constexpr (std::is_same_v<MatrixType, Moment::SymbolicMatrix>) {
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
            const auto [actual_offset, did_insertion] = this->indices.insert(index, matrix_offset);
            assert(actual_offset == matrix_offset);
            assert(did_insertion);
            matrixFactory.notify(lock, index, actual_offset, matrix_ref);

            return std::pair<size_t, matrix_t&>{static_cast<size_t>(matrix_offset), matrix_ref};
        }

        /**
         * Register existing matrix at specified index
         */
        [[nodiscard]] ptrdiff_t insert_alias(const MaintainsMutex::WriteLock& lock,
                                             const index_t& index, const ptrdiff_t matrix_offset) {
            // Must hold write lock
            assert(this->system.is_locked_write_lock(lock));
            assert(matrix_offset>=0);

            // Put into indices
            const auto [actual_offset, did_insertion] = this->indices.insert(index, matrix_offset);

            return actual_offset;
        }

        /**
         * Retrieve matrix with requested index.
         * @param index The description of the matrix.
         * @return Reference to the found matrix.
         * @throws Moment::errors::missing_component if no matrix exists at index.
         */
        [[nodiscard]] const MatrixType& find(const MaintainsMutex::ReadLock& lock, const index_t& index) const {
            assert(this->system.is_locked_read_lock(lock));

            auto existing_index = this->indices.find(index);
            if (existing_index < 0) {
                throw Moment::errors::missing_component(index.to_string(this->system).append(" was not found."));
            }

            if constexpr(std::is_same_v<MatrixType, Moment::SymbolicMatrix>) {
                return this->system[existing_index]; // throws if bad offset.
            } else {
                return dynamic_cast<const MatrixType&>(this->system.get(existing_index));// throws if bad offset / cast
            }
        }

        [[nodiscard]] inline ptrdiff_t find_index(const index_t& index) const noexcept {
            return this->indices.find(index);
        }

        [[nodiscard]] inline bool contains(const index_t& index) const noexcept {
            return this->indices.contains(index);
        }

        [[nodiscard]] inline const MatrixType& operator()(const index_t& index) const {
            auto read_lock = this->system.get_read_lock();
            return this->find(read_lock, index);
            // ~read_lock
        }

        [[nodiscard]] inline const MatrixType& operator()(const index_t& index,
                                                    const MTPolicy mt_policy = MTPolicy::Optional) {
            // Attempt to access with read lock
            auto read_lock = this->system.get_read_lock();
            auto existing_index = this->indices.find(index);
            if (existing_index >= 0) {
                try {
                    if constexpr (std::is_same_v<MatrixType, Moment::SymbolicMatrix>) {
                        return this->system.get(existing_index);
                    } else {
                        return dynamic_cast<MatrixType&>(this->system.get(existing_index));
                    }
                } catch (const errors::missing_component& mce) {
                    throw std::runtime_error{"Index for matrix was found, but matrix was invalid."};
                } catch (const std::bad_cast& bad_cast) {
                    throw std::runtime_error{"Index for matrix was found, but matrix was of invalid type."};
                }
            }

            // Did not find with read lock, so move on to creation:
            read_lock.unlock();
            auto write_lock = this->system.get_write_lock();
            auto [offset, matrix] = this->create(write_lock, index, mt_policy);
            return matrix; // ~write_lock
        }

    };
}
