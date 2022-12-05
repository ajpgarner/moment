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
    class CanonicalObservables;
    class InflationExplicitSymbolIndex;

    class InflationMatrixSystem : public MatrixSystem {

    private:
        class InflationContext &inflationContext;

        std::unique_ptr<FactorTable> factors;

        std::unique_ptr<class CanonicalObservables> canonicalObservables;

        std::unique_ptr<InflationExplicitSymbolIndex> explicitSymbols;

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


        /**
         * Get factorization list associated with matrices.
         */
        const class CanonicalObservables& CanonicalObservables() const noexcept { return *this->canonicalObservables; }

        /**
        * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
        * the context (including joint measurements).
        * @throws errors::missing_compoment if not generated.
        */
        [[nodiscard]] const InflationExplicitSymbolIndex& ExplicitSymbolTable() const;

        /**
         * Calculates the longest real sequences that can exist within this system (i.e. the highest number of
         *  parties, all of whose joint measurement outcomes correspond to symbols within.).
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] size_t MaxRealSequenceLength() const noexcept;

    protected:
        void onNewMomentMatrixCreated(size_t level, const class MomentMatrix &mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const class LocalizingMatrix &lm) override;

    };

}