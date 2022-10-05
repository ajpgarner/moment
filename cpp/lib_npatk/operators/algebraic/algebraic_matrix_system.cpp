/**
 * algebraic_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_matrix_system.h"
#include "algebraic_context.h"

#include "../../symbolic/symbol_set.h"


namespace NPATK {


    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<AlgebraicContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<AlgebraicContext&>(this->getContext())} {

    }

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<AlgebraicContext&>(this->getContext())} {

    }

    void AlgebraicMatrixSystem::generate_aliases(size_t stringLength) {
        // Get write lock
        auto lock = this->getWriteLock();

        // Generate raw sequences
        this->algebraicContext.generate_aliases(stringLength);
    }

}
