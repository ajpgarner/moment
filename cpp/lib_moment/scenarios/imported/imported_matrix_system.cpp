/**
 * imported_matrix_system.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "imported_matrix_system.h"
#include "imported_context.h"

namespace Moment::Imported {
ImportedMatrixSystem::ImportedMatrixSystem()
        : MatrixSystem(std::make_unique<ImportedContext>()) {

    }

    void ImportedMatrixSystem::beforeNewMomentMatrixCreated(size_t level) {
        throw std::runtime_error{"Operator matrices cannot be procedurally generated in imported context."};
    }

    void ImportedMatrixSystem::beforeNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi) {
        throw std::runtime_error{"Operator matrices cannot be procedurally generated in imported context."};
    }
}