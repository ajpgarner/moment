/**
 * inflation_matrix_system.h
 * 
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#pragma once

#include <memory>

#include "matrix_system.h"
#include "utilities/index_tree.h"

namespace Moment::Inflation {

    class InflationContext;
    class FactorTable;
    class CanonicalObservables;
    class ExtendedMatrix;
    class InflationExplicitSymbolIndex;
    class InflationImplicitSymbols;

    class InflationMatrixSystem : public MatrixSystem {
    private:
        class InflationContext &inflationContext;

        std::unique_ptr<FactorTable> factors;

        std::unique_ptr<class CanonicalObservables> canonicalObservables;

        std::unique_ptr<InflationExplicitSymbolIndex> explicitSymbols;

        std::unique_ptr<InflationImplicitSymbols> implicitSymbols;

        IndexTree<symbol_name_t, size_t> extension_indices;

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

        std::string system_type_name() const override {
            return "Inflation Matrix System";
        }

        /**
         * Get algebraic version of context object
         */
        const class InflationContext& InflationContext() const noexcept { return this->inflationContext; }

        /**
         * Get factorization list associated with matrices.
         * For thread safety, call for read lock before accessing.
         */
        const FactorTable& Factors() const noexcept { return *this->factors; }

        /**
         * Get write access to factorization list associated with matrices.
         * For thread safety, call for write lock before making changes.
         */
        FactorTable& Factors() noexcept { return *this->factors; }

        /**
         * Create or retrieve an extended matrix. This function will call for a write lock.
         */
        std::pair<size_t, ExtendedMatrix&> create_extended_matrix(const class MomentMatrix& source,
                                                                  std::span<const symbol_name_t> extensions);

        /**
         * Retrieve index of extended matrix, that already exists, by extensions.
         */
        ptrdiff_t find_extended_matrix(size_t mm_level, std::span<const symbol_name_t> extensions);

        /**
         * Get factorization list associated with matrices.
         */
        const class CanonicalObservables& CanonicalObservables() const noexcept { return *this->canonicalObservables; }

        /**
        * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
        * the context (including joint measurements).
        * @throws errors::missing_component if not generated.
        */
        [[nodiscard]] const InflationExplicitSymbolIndex& ExplicitSymbolTable() const;

        /**
        * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
        * the context (including joint measurements).
        * @throws errors::missing_component if not generated.
        */
        [[nodiscard]] const InflationImplicitSymbols& ImplicitSymbolTable() const;

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