/**
 * locality_probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/probability_tensor.h"


namespace Moment::Locality {
    class LocalityContext;

    class LocalityProbabilityTensor : public ProbabilityTensor {
    public:
        LocalityProbabilityTensor(const CollinsGisin& cg, const LocalityContext& context);

    };
}