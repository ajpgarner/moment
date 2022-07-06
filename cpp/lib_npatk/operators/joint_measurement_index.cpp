/**
 * joint_measurement_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "joint_measurement_index.h"
#include "context.h"

namespace NPATK {
    JointMeasurementIndex::JointMeasurementIndex(const NPATK::Context &context)
        : JointMeasurementIndex(context.measurements_per_party()) { }
}
