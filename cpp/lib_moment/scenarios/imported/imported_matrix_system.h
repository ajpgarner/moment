/**
 * imported_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix_system/matrix_system.h"
#include "imported_context.h"

#include "symbolic/monomial.h"
#include "tensor/square_matrix.h"

#include <memory>
#include <string>
#include <stdexcept>


namespace Moment::Imported {
    class ImportedContext;

    namespace errors {
        struct bad_import_matrix : std::runtime_error {
            bad_import_matrix(const std::string& what) : std::runtime_error{what} { }
        };
    }

    /**
     * Matrix system with no underlying operators - just symbols.
     */
    class ImportedMatrixSystem : public MatrixSystem {
    public:
        ImportedContext& importedContext;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        ImportedMatrixSystem(bool purely_real = false);

        /**
         * Register a matrix into the system, identifying symbols, etc. Call for write lock before importing.
         * @param input The input matrix.
         * @param is_complex True if some aspect of the matrix represents complex components
         * @param is_hermitian True if the matrix is self-adjoint (real symmetric, or complex Hermitian).
         * @return The index of the newly inserted matrix, and a reference to it in situ.
         * Do not call for read lock before importing matrix! System will call its own write lock.
         */
        std::pair<size_t, class Matrix&>
        import_matrix(std::unique_ptr<SquareMatrix<Monomial>> input, bool is_complex, bool is_hermitian);

        std::string system_type_name() const override {
            return "Imported Matrix System";
        }

    protected:
        std::unique_ptr<class Matrix>
        createNewMomentMatrix(WriteLock& lock, size_t level, Multithreading::MultiThreadPolicy mt_policy) override;

        std::unique_ptr<class Matrix>
        createNewLocalizingMatrix(WriteLock& lock, const LocalizingMatrixIndex &lmi,
                                  Multithreading::MultiThreadPolicy mt_policy) override;

        std::unique_ptr<class PolynomialMatrix>
        createNewPolyLM(MaintainsMutex::WriteLock &lock, const PolynomialLMIndex &index,
                        Multithreading::MultiThreadPolicy mt_policy) override;

    };

}