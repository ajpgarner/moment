/**
 * inflation_matrix_system.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <memory>
#include <set>

#include "matrix_system/matrix_system.h"
#include "probability/maintains_tensors.h"
#include "utilities/index_tree.h"

namespace Moment {
    class CollinsGisin;
    class MonomialMatrix;
    class ProbabilityTensor;
}

namespace Moment::Inflation {
    class CanonicalObservables;
    class ExtendedMatrix;
    class ExtensionSuggester;
    class FactorTable;
    class InflationCollinsGisin;
    class InflationContext;
    class InflationProbabilityTensor;


    class InflationMatrixSystem : public MaintainsTensors {
    private:
        class InflationContext &inflationContext;

        std::unique_ptr<FactorTable> factors;

        std::unique_ptr<class CanonicalObservables> canonicalObservables;

        std::unique_ptr<ExtensionSuggester> extensionSuggester;

        IndexTree<symbol_name_t, size_t> extension_indices;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<class InflationContext> context, double zero_tolerance = 1.0);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit InflationMatrixSystem(std::unique_ptr<class Context> context, double zero_tolerance = 1.0);

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
        std::pair<size_t, ExtendedMatrix&> create_extended_matrix(const MonomialMatrix& source,
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
         * Returns an indexing in the Collins-Gisin ordering with additional inflation-scenario functionality.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class InflationCollinsGisin& InflationCollinsGisin() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements). Includes inflation-related functionality.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class InflationProbabilityTensor& InflationProbabilityTensor() const;

        /**
         * Calculates the longest real sequences that can exist within this system (i.e. the highest number of
         *  parties, all of whose joint measurement outcomes correspond to symbols within.).
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] size_t MaxRealSequenceLength() const noexcept;

        /**
         * Suggest scalar extensions to impose factorization constraints on a matrix
         */
        [[nodiscard]] std::set<symbol_name_t> suggest_extensions(const class MonomialMatrix& matrix) const;

    protected:
        void onNewMomentMatrixCreated(size_t level, const class Matrix &mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const class Matrix &lm) override;

        void onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) override;

    private:
        std::unique_ptr<class CollinsGisin> makeCollinsGisin() override;

        std::unique_ptr<class ProbabilityTensor> makeProbabilityTensor() override;

    };

}