/**
 * joint_measurement_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "joint_measurement_index.h"
#include "locality_context.h"

#include <algorithm>

namespace NPATK {
    JointMeasurementIndex::JointMeasurementIndex(const LocalityContext &context, size_t max_depth)
        : JointMeasurementIndex(context.measurements_per_party(), std::min(max_depth, context.Parties.size())) { }
}
