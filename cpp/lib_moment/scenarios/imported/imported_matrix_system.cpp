/**
 * imported_matrix_system.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "imported_matrix_system.h"
#include "imported_context.h"

#include "matrix/symbolic_matrix.h"

#include <cassert>

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

    size_t ImportedMatrixSystem::import_matrix(std::unique_ptr<SquareMatrix<SymbolExpression>> input) {
        assert(input);

        // Parse for largest symbol identity
        size_t new_max_symbol_id = 0;
        for (const auto& symbol_expression : *input) {
            if (symbol_expression.id > new_max_symbol_id) {
                new_max_symbol_id = symbol_expression.id;
            }
        }

        // Densely fill table until we reach largest supplied symbol
        while (this->Symbols().size() <= new_max_symbol_id) {
            this->Symbols().create();
        }

        // Construct symbolic matrix
        return this->push_back(std::make_unique<SymbolicMatrix>(this->Context(), this->Symbols(), std::move(input)));
    }
}