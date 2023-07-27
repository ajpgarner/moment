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

#include <shared_mutex>
#include <string>
#include <utility>


namespace Moment {

    class Matrix;
    class PolynomialMatrix;
    class MatrixSystem;

    using MatrixSystemWriteLock = std::unique_lock<std::shared_mutex>;

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

        [[nodiscard]] std::pair<ptrdiff_t, Matrix&>
        operator()(MatrixSystemWriteLock& lock, Index level, Multithreading::MultiThreadPolicy mt_policy);

        void notify(size_t index, Matrix& matrix);

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

        [[nodiscard]] std::pair<ptrdiff_t, Matrix&>
        operator()(MatrixSystemWriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const Index& lmi, Matrix& matrix);

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
        operator()(MatrixSystemWriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const Index& index, PolynomialMatrix& matrix);

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

        [[nodiscard]] std::pair<ptrdiff_t, Matrix&>
        operator()(MatrixSystemWriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const Index& index, Matrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };

}