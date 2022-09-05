/**
 * matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_system.h"


namespace NPATK {
    namespace {
        Context& assertContext(std::shared_ptr<Context> ctxtIn) {
            assert(ctxtIn);
            return *ctxtIn;
        }
    }

    MatrixSystem::MatrixSystem(std::shared_ptr<Context> ctxtIn)
        : context{std::move(ctxtIn)}, symbol_table{assertContext(context)} {

    }

    const MomentMatrix &MatrixSystem::MomentMatrix(size_t level) const {
        assert(this->HasLevel(level));
        return *momentMatrices[level];
    }

    MomentMatrix& MatrixSystem::CreateMomentMatrix(size_t level) {
        if (this->HasLevel(level)) {
            return *momentMatrices[level];
        }

        // Fill with null elements if some are missing
        if (this->momentMatrices.size() < level+1) {
            this->momentMatrices.resize(level+1);
        }

        // Generate new moment matrix
        this->momentMatrices[level] = std::make_unique<class MomentMatrix>(*this->context, this->symbol_table, level);

        return *this->momentMatrices[level];
    }
}