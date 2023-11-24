/**
 * matrix_factories.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "localizing_matrix_index.h"
#include "polynomial_localizing_matrix_index.h"

#include "multithreading/multithreading.h"
#include "multithreading/maintains_mutex.h"

#include <shared_mutex>
#include <string>
#include <utility>


namespace Moment {

    class SymbolicMatrix;
    class PolynomialMatrix;
    class MatrixSystem;

    /**
     * Factory: makes moment matrices
     */
    class MomentMatrixFactory {
    public:
        using Index = size_t;

    private:
        MatrixSystem& system;

    public:
        explicit MomentMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, Index level, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, size_t index, ptrdiff_t offset, SymbolicMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(Index level) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };

    /**
     * Factory: makes monomial localizing matrices.
     */
    class LocalizingMatrixFactory {
    public:
        using Index = LocalizingMatrixIndex;

    private:
        MatrixSystem& system;

    public:
        explicit LocalizingMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& lmi, ptrdiff_t offset, SymbolicMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& lmi) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };

    /**
      * Factory: makes polynomial localizing matrices.
      */
    class PolynomialLocalizingMatrixFactory  {
    public:
        using Index = PolynomialLMIndex;

    private:
        MatrixSystem& system;

    public:
        explicit PolynomialLocalizingMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, PolynomialMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const PolynomialLMIndex& pmi) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };

    /**
     * Factory: makes substituted matrices.
     */
    class SubstitutedMatrixFactory {
    public:
        using Index = std::pair<ptrdiff_t, ptrdiff_t>;

    private:
        MatrixSystem& system;

    public:
        explicit SubstitutedMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, SymbolicMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };

}