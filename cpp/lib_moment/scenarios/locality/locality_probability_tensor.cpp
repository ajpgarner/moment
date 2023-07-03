/**
 * locality_probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_probability_tensor.h"
#include "locality_context.h"

namespace Moment::Locality {

    namespace {

        ProbabilityTensor::ConstructInfo make_construct_info(const LocalityContext& context) {
            ProbabilityTensor::ConstructInfo info;
            info.totalDimensions = context.outcomes_per_party();
            const auto& mpp = context.measurements_per_party();
            std::copy(mpp.cbegin(), mpp.cend(), std::back_inserter(info.mmtsPerParty));
            info.outcomesPerMeasurement = context.outcomes_per_measurement();
            return info;
        }

    }

    LocalityProbabilityTensor::LocalityProbabilityTensor(const CollinsGisin &cg, const LocalityContext &context)
        :  ProbabilityTensor(cg, make_construct_info(context)) {

    }
}