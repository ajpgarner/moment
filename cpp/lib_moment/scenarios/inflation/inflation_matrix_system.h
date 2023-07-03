/**
 * inflation_matrix_system.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <memory>
#include <set>

#include "matrix_system.h"
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


    class InflationMatrixSystem : public MatrixSystem {
    private:
        class InflationContext &inflationContext;

        std::unique_ptr<FactorTable> factors;

        std::unique_ptr<class CanonicalObservables> canonicalObservables;

        std::unique_ptr<ExtensionSuggester> extensionSuggester;

        std::unique_ptr<InflationCollinsGisin> collinsGisin;

        std::unique_ptr<InflationProbabilityTensor> probabilityTensor;

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
         * Returns an indexing in the Collins-Gisin ordering.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class CollinsGisin& CollinsGisin() const;

        /**
         * Checks if it is necessary to refresh the explicit symbol table, and refresh it if so.
         * If a refresh is necessary msReadLock will be released, and system will wait for write lock. Read-lock will be
         * reacquired after write is complete.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshCollinsGisin(std::shared_lock<std::shared_mutex>& read_lock);

        /**
         * Checks if it is necessary to refresh the explicit symbol table, and refresh it if so.
         * Acquires write-lock if refresh is necessary.  Either release read-locks before calling, or use the overload
         * with a read-lock parameter.
         * @return
         */
        bool RefreshCollinsGisin();

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements).
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class ProbabilityTensor& ProbabilityTensor() const;

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

    };

}