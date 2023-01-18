/**
 * imported_matrix_system.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "matrix_system.h"
#include "imported_context.h"

#include "utilities/square_matrix.h"
#include "symbolic/symbol_expression.h"
#include <memory>
#include <stdexcept>


namespace Moment::Imported {
    class ImportedContext;

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
        size_t import_matrix(std::unique_ptr<SquareMatrix<SymbolExpression>> input);

    protected:
        void beforeNewMomentMatrixCreated(size_t level) override;

        void beforeNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi) override;


    };

}