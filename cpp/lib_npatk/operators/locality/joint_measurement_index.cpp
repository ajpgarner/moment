/**
 * joint_measurement_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "joint_measurement_index.h"
#include "locality_context.h"
#include "operators/inflation/inflation_context.h"

#include <algorithm>
#include <vector>

namespace NPATK {
    JointMeasurementIndex::JointMeasurementIndex(const LocalityContext &context, size_t max_depth)
        : JointMeasurementIndex(context.measurements_per_party(), std::min(max_depth, context.Parties.size())) { }

    JointMeasurementIndex::JointMeasurementIndex(const InflationContext &context, size_t max_depth)
        : JointMeasurementIndex(std::vector<size_t>(context.Observables().size(), static_cast<size_t>(1)),
                                std::min(max_depth, context.Observables().size())) { }
}
