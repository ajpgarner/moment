/**
 * derived_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "derived_matrix_indices.h"

#include "matrix_system/matrix_system.h"

#include "multithreading/multithreading.h"

#include <memory>
#include <stdexcept>

namespace Moment::errors {
    class bad_transformation_error : public std::runtime_error {
    public:
        explicit bad_transformation_error(const std::string& what) : std::runtime_error{what} { }
    };

    class too_large_to_transform_error : public bad_transformation_error {
    public:
        too_large_to_transform_error(size_t max_size, size_t requested_size, const std::string& object_name);
    };
}

namespace Moment::Derived {
    class DerivedContext;
    class SymbolTableMap;

    class DerivedMatrixSystem : public MatrixSystem {
    public:
        /**
         * Virtual factory class for making the symbol table map.
         * The purpose of this class is to inject a virtual function call in the constructor of DerivedMatrixSystem.
         * Implementations only need to support operator() being called once.
         */
        class STMFactory {
        protected:
            STMFactory() noexcept = default;

            virtual ~STMFactory() noexcept = default;

        public:
            [[nodiscard]] virtual std::unique_ptr<SymbolTableMap>
            operator()(SymbolTable& origin, SymbolTable& target, Multithreading::MultiThreadPolicy mt_policy) = 0;
        };

    private:
        class DerivedContext& derived_context;

        /**
         * Owning pointer to base system.
         * Ownership is necessary, to prevent deletion of base system while SMS is still alive.
         */
        std::shared_ptr<MatrixSystem> base_ms_ptr;

        /** Map that defines the system */
        std::unique_ptr<Derived::SymbolTableMap> map_ptr;

    public:
        /** Index to derived matrices, by source matrix index */
        DerivedMatrixIndices DerivedMatrices;

    public:
        explicit DerivedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system, STMFactory&& stmf,
                                     double tolerance = -1.0,
                             Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        virtual ~DerivedMatrixSystem() noexcept;

        /**
         * Gets the longest words in the base system that are sure to be mapped into this symbol.
         * The check can be skipped, but then this risks individual symbols triggering error::bad_map errors at a later
         * stage in the symbol matrix generations.
         */
        virtual size_t longest_supported_word() const noexcept { return std::numeric_limits<size_t>::max(); }

        /**
         * The original system this DMS is derived from.
         */
        [[nodiscard]] inline MatrixSystem& base_system() noexcept {
            return *base_ms_ptr;
        }

        /**
         * The original system this DMS is derived from.
         */
        [[nodiscard]] inline const MatrixSystem& base_system() const noexcept {
            return *base_ms_ptr;
        }

        /**
         * Map between base matrix system symbols and this system's symbols.
         */
        [[nodiscard]] inline const Derived::SymbolTableMap& map() const {
            return *this->map_ptr;
        }

        [[nodiscard]] std::string system_type_name() const override {
            return "Derived Matrix System";
        }

        /**
         * A description block for the map that defines this SMS.
         * For thread safety, a read lock should be in place on this matrix system, AND on the base matrix system.
         */
        [[nodiscard]] virtual std::string describe_map() const;

        /** Reference to derived context object. */
        const class DerivedContext& DerivedContext() const noexcept {
            return this->derived_context;
        }

    protected:
        [[nodiscard]] std::unique_ptr<class SymbolicMatrix>
        create_moment_matrix(const WriteLock& lock, size_t level, Multithreading::MultiThreadPolicy mt_policy) override;

        [[nodiscard]] std::unique_ptr<class SymbolicMatrix>
        create_localizing_matrix(const WriteLock& lock, const LocalizingMatrixIndex &lmi,
                                 Multithreading::MultiThreadPolicy mt_policy) override;

        [[nodiscard]] std::unique_ptr<class PolynomialMatrix>
        create_polynomial_localizing_matrix(const MaintainsMutex::WriteLock &lock,
                                            const PolynomialLocalizingMatrixIndex &index,
                                            Multithreading::MultiThreadPolicy mt_policy) override;

        /**
         * Generically create symmetrization of a matrix.
         */
        [[nodiscard]] virtual std::unique_ptr<class SymbolicMatrix>
        create_derived_matrix(const WriteLock& lock, ptrdiff_t source_offset, Multithreading::MultiThreadPolicy mt_policy);

        void on_new_moment_matrix(const WriteLock& write_lock, size_t level, ptrdiff_t matrix_offset,
                                  const SymbolicMatrix& mm) override;

        void on_new_localizing_matrix(const WriteLock& write_lock, const LocalizingMatrixIndex& lmi,
                                      ptrdiff_t matrix_offset, const SymbolicMatrix& lm) override;

        void on_new_polynomial_localizing_matrix(const WriteLock& write_lock, const PolynomialLocalizingMatrixIndex& lmi,
                                                 ptrdiff_t matrix_offset, const PolynomialMatrix& plm) override;

        /**
         * Called when a symmetrized matrix is generically created.
         */
        virtual void on_new_derived_matrix(const WriteLock& write_lock,
                                           ptrdiff_t source_offset, ptrdiff_t  target_offset,
                                           const SymbolicMatrix& target_matrix);

    protected:
        static std::unique_ptr<class Context> make_derived_context(const MatrixSystem& source_system);

        friend class DerivedMatrixFactory;
    };
}
