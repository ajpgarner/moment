/**
 * imported_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix_system.h"
#include "imported_context.h"

#include "matrix/matrix_type.h"
#include "symbolic/monomial.h"
#include "utilities/square_matrix.h"

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
         * @return The index of the newly inserted matrix.
         */
        size_t import_matrix(std::unique_ptr<SquareMatrix<Monomial>> input, MatrixType matrix_type);

        std::string system_type_name() const override {
            return "Imported Matrix System";
        }

    protected:
        std::unique_ptr<class Matrix> createNewMomentMatrix(size_t level) override;

        std::unique_ptr<class Matrix> createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) override;

    };

}