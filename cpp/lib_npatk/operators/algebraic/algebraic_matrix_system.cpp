/**
 * algebraic_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "algebraic_matrix_system.h"
#include "algebraic_context.h"

#include "../../symbolic/symbol_set.h"


namespace NPATK {


    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class AlgebraicContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {

    }

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {

    }

    void AlgebraicMatrixSystem::generate_aliases(size_t stringLength) {
        // Get write lock
        auto lock = this->get_write_lock();

        // Generate raw sequences
        this->algebraicContext.generate_aliases(stringLength);
    }

    void AlgebraicMatrixSystem::beforeNewMomentMatrixCreated(size_t level) {
        this->algebraicContext.generate_aliases(level*2);
    }

    void AlgebraicMatrixSystem::beforeNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi) {
        this->algebraicContext.generate_aliases((lmi.Level*2) + lmi.Word.size());
    }

}
