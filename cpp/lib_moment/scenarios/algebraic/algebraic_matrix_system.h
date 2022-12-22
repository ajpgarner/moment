/**
 * algebraic_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "matrix_system.h"

namespace Moment::Algebraic {
    class AlgebraicContext;

    class AlgebraicMatrixSystem : public MatrixSystem {

    private:
        class AlgebraicContext &algebraicContext;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<class AlgebraicContext> context);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<class Context> context);

        /**
         * Get algebraic version of context object
         */
        const class AlgebraicContext& AlgebraicContext() const noexcept { return this->algebraicContext; }

    };
}