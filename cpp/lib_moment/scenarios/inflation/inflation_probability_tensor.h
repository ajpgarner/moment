/**
 * inflation_probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "probability/probability_tensor.h"
#include "observable_variant_index.h"


namespace Moment::Inflation {
    class InflationMatrixSystem;
    class InflationContext;

    class InflationProbabilityTensor : public ProbabilityTensor {
    public:
        const InflationContext& context;

    public:
        explicit InflationProbabilityTensor(const InflationMatrixSystem& system);


        [[nodiscard]] ProbabilityTensorRange measurement_to_range(std::span<const OVIndex> mmtIndices) const;

        [[nodiscard]] ProbabilityTensorRange measurement_to_range(std::span<const OVIndex> freeMeasurements,
                                                                  std::span<const OVOIndex> fixedOutcomes) const;

        [[nodiscard]] ProbabilityTensor::ElementView outcome_to_element(std::span<const OVOIndex> fixedOutcomes) const;
    };
}