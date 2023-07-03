/**
 * inflation_probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_probability_tensor.h"

#include "inflation_context.h"

namespace Moment::Inflation {
    namespace {
        ProbabilityTensor::ConstructInfo make_construct_info(const InflationContext& context) {
            throw std::runtime_error{"PT not yet supported for inflation context."};
        }
    }

    InflationProbabilityTensor::InflationProbabilityTensor(const CollinsGisin &cg, const InflationContext &context)
        : ProbabilityTensor(cg, make_construct_info(context)) {

    }

}