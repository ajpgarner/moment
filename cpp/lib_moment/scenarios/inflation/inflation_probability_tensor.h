/**
 * inflation_probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/probability_tensor.h"

namespace Moment::Inflation {
    class InflationMatrixSystem;

    class InflationProbabilityTensor : public ProbabilityTensor {
    public:
        explicit InflationProbabilityTensor(const InflationMatrixSystem& system);
    };
}