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

#include "extended_matrix_indices.h"

namespace Moment {
    class CollinsGisin;
    class Symbol;
    class MonomialMatrix;
    class ProbabilityTensor;
}

namespace Moment::Inflation {
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

        std::unique_ptr<ExtensionSuggester> extensionSuggester;

    public:
        ExtendedMatrixIndices ExtendedMatrices;

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
         * Suggest scalar extensions to impose factorization constraints on a matrix
         */
        [[nodiscard]] std::set<symbol_name_t> suggest_extensions(const class MonomialMatrix& matrix) const;

    protected:
        /**
         * Virtual method, called to generate an extended matrix.
         * @param lmi The hierarchy Level and word that describes the localizing matrix.
         * @param mt_policy Is multithreaded creation used?
         * @return Owning pointer of new localizing matrix.
         */
        virtual std::unique_ptr<class ExtendedMatrix>
        createNewExtendedMatrix(MaintainsMutex::WriteLock &lock, const ExtendedMatrixIndex& index,
                                Multithreading::MultiThreadPolicy mt_policy);

        void onNewMomentMatrixCreated(size_t level, const class Matrix &mm) override;

        void onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const class Matrix &lm) override;

        virtual void onNewExtendedMatrixCreated(const ExtendedMatrixIndex&, const class ExtendedMatrix& em);

        void onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) override;


    private:
        std::unique_ptr<class CollinsGisin> makeCollinsGisin() override;

        std::unique_ptr<class ProbabilityTensor> makeProbabilityTensor() override;

        friend class ExtendedMatrixFactory;
        friend ExtendedMatrixIndices;
    };

}