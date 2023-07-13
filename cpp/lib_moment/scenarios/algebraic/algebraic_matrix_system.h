/**
 * algebraic_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system/matrix_system.h"

namespace Moment::Algebraic {
    class AlgebraicContext;

    class AlgebraicMatrixSystem : public MatrixSystem {

    private:
        class AlgebraicContext &algebraicContext;

    public:
        std::string system_type_name() const override {
            return "Algebraic Matrix System";
        }

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         * @param zero_tolerance The multiplier of epsilon below which, we treat a floating point number as zero.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<class AlgebraicContext> context,
                                       double zero_tolerance = 1.0);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         * @param zero_tolerance The multiplier of epsilon below which, we treat a floating point number as zero.
         */
        explicit AlgebraicMatrixSystem(std::unique_ptr<class Context> context,
                                       double zero_tolerance = 1.0);

        /**
         * Get algebraic version of context object
         */
        const class AlgebraicContext& AlgebraicContext() const noexcept { return this->algebraicContext; }

    };
}