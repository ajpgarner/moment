/**
 * locality_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system.h"

namespace Moment {
    class CollinsGisin;
    class ProbabilityTensor;
}

namespace Moment::Locality {
    class LocalityCollinsGisin;
    class LocalityContext;
    class LocalityProbabilityTensor;

    class LocalityMatrixSystem : public MatrixSystem {
    public:
        const class LocalityContext& localityContext;

    private:
        /** Map of explicitly specified probabilities, given by Collins Gisin index */
        std::unique_ptr<LocalityCollinsGisin> collinsGisin;

        /** Map of implied probabilities */
        std::unique_ptr<LocalityProbabilityTensor> probabilityTensor;

        /** XXX: To remove */
        size_t maxProbabilityLength = 0;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit LocalityMatrixSystem(std::unique_ptr<class LocalityContext> context, double tolerance = 1.0);

        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit LocalityMatrixSystem(std::unique_ptr<class Context> context, double tolerance = 1.0);

        ~LocalityMatrixSystem() noexcept override;

        std::string system_type_name() const override {
            return "Locality Matrix System";
        }

        /**
         * Calculates the longest real sequences that can exist within this system (i.e. the highest number of
         *  parties, all of whose joint measurement outcomes correspond to symbols within.).
         * For thread safety, call for a read lock first.
         */
        [[nodiscard]] size_t MaxRealSequenceLength() const noexcept;


        /**
         * Returns an indexing in the Collins-Gisin ordering.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class CollinsGisin& CollinsGisin() const;

        /**
         * Returns an indexing in the Collins-Gisin ordering, with additional locality-specific functions.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalityCollinsGisin& LocalityCollinsGisin() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements).
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class ProbabilityTensor& ProbabilityTensor() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements). Includes locality-indexing options.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalityProbabilityTensor& LocalityProbabilityTensor() const;

        /**
         * Checks if it is necessary to refresh the explicit Collins-Gisin table, and refresh it if so.
         * If a refresh is necessary msReadLock will be released, and system will wait for write lock. Read-lock will be
         * reacquired after write is complete.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshCollinsGisin(std::shared_lock<std::shared_mutex>& read_lock);

        /**
         * Checks if it is necessary to refresh the explicit symbol table, and refresh it if so.
         * Acquires write-lock if refresh is necessary: either release locks before calling, or use the overload
         * with a read-lock parameter.
         * @return
         */
        bool RefreshCollinsGisin();

        /**
         * Checks if it is necessary to refresh the implicit probability symbol table, and refresh it if so.
         * If a refresh is necessary msReadLock will be released, and system will wait for write lock. Read-lock will be
         * reacquired after write is complete.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshProbabilityTensor(std::shared_lock<std::shared_mutex>& read_lock);

        /**
         * Checks if it is necessary to refresh the implicit probability symbol table, and refresh it if so.
         * Acquires write-lock if refresh is necessary: either release locks before calling, or use the overload
         * with a read-lock parameter.
         * @return True if explicit symbol table is complete.
         */
        bool RefreshProbabilityTensor();



    protected:
        void onNewMomentMatrixCreated(size_t level, const class Matrix& mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const Matrix &lm) override;

        void onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) override;
    };
}