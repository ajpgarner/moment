/**
 * locality_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system.h"

namespace Moment {
    class ExplicitSymbolIndex;
}

namespace Moment::Locality {
    class LocalityContext;
    class LocalityImplicitSymbols;
    class CollinsGisin;

    class LocalityMatrixSystem : public MatrixSystem {
    public:
        const class LocalityContext& localityContext;

    private:
        /** Map of measurement outcome symbols */
        std::unique_ptr<ExplicitSymbolIndex> explicitSymbols;

        /** Map of implied probabilities */
        std::unique_ptr<LocalityImplicitSymbols> implicitSymbols;

        /** Map of outcome symbols, by Collins Gisin index */
        std::unique_ptr<class CollinsGisin> collinsGisin;


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
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const class CollinsGisin& CollinsGisin() const;

        /**
         * Returns an indexing of real-valued symbols that correspond to explicit operators/operator sequences within
         * the context (including joint measurements).
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const ExplicitSymbolIndex& ExplicitSymbolTable() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements).
         * @throws errors::missing_compoment if not generated.
         */
        [[nodiscard]] const LocalityImplicitSymbols& ImplicitSymbolTable() const;

    protected:
        void onNewMomentMatrixCreated(size_t level, const class Matrix& mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const Matrix &lm) override;

        void onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) override;
    };
}