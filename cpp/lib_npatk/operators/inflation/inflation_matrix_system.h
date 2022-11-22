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
    class FactorTable;

    class InflationMatrixSystem : public MatrixSystem {

    private:
        class InflationContext &inflationContext;

        std::unique_ptr<FactorTable> factors;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<class InflationContext> context);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<class Context> context);

        /** Destructor */
        ~InflationMatrixSystem() noexcept override;

        /**
         * Get algebraic version of context object
         */
        const class InflationContext& InflationContext() const noexcept { return this->inflationContext; }

        /**
         * Get factorization list associated with matrices.
         */
        const FactorTable& Factors() const noexcept { return *this->factors; }

    protected:
        void onNewMomentMatrixCreated(size_t level, const class MomentMatrix &mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const class LocalizingMatrix &lm) override;

    };

}