/**
 * locality_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix_system/matrix_system.h"
#include "probability/maintains_tensors.h"

namespace Moment::Locality {
    class LocalityCollinsGisin;
    class LocalityContext;
    class LocalityProbabilityTensor;

    class LocalityMatrixSystem : public MaintainsTensors {
    public:
        const class LocalityContext& localityContext;

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
         * Returns an indexing in the Collins-Gisin ordering, with additional locality-specific functions.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalityCollinsGisin& LocalityCollinsGisin() const;

        /**
         * Returns an indexing of all correlators, if the scenario is composed of binary measurements.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalityFullCorrelator& LocalityFullCorrelator() const;

        /**
         * Returns an indexing of all real-valued symbols, including those from ExplicitSymbolTable(), but also implied
         * "final" outcomes of measurements (including joint measurements). Includes locality-indexing options.
         * @throws errors::missing_component if not generated.
         */
        [[nodiscard]] const class LocalityProbabilityTensor& LocalityProbabilityTensor() const;

        bool CanHaveFullCorrelator() const noexcept override;

    private:
        std::unique_ptr<class CollinsGisin> makeCollinsGisin() override;

        std::unique_ptr<class FullCorrelator> makeFullCorrelator() override;

        std::unique_ptr<class ProbabilityTensor> makeProbabilityTensor() override;
    };
}