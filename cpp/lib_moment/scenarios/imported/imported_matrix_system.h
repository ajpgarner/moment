/**
 * imported_matrix_system.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "matrix_system.h"
#include "imported_context.h"

#include <stdexcept>

namespace Moment::Imported {
    class ImportedContext;

    /**
     * Matrix system with no underlying operators - just symbols.
     */
    class ImportedMatrixSystem : public MatrixSystem {
    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        ImportedMatrixSystem();

    protected:
        void beforeNewMomentMatrixCreated(size_t level) override;

        void beforeNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi) override;


    };

}