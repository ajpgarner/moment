/**
 * inflation_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <memory>

#include "../matrix_system.h"

namespace NPATK {

    class InflationContext;

    class InflationMatrixSystem : public MatrixSystem {

    private:
        class InflationContext &inflationContext;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<InflationContext> context);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<class Context> context);

        /**
         * Get algebraic version of context object
         */
        const class InflationContext &InflationContext() const noexcept { return this->inflationContext; }

    };

}