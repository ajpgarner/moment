/**
 * joint_measurement_index.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "utilities/recursive_index.h"

namespace Moment {

    class JointMeasurementIndex : public MonotonicChunkRecursiveStorage<
                                            std::pair<ptrdiff_t, ptrdiff_t>, JointMeasurementIndex> {
    public:
        constexpr explicit JointMeasurementIndex(std::span<const size_t> chunk_sizes,
                                                     size_t max_depth,
                                                     std::pair<ptrdiff_t, ptrdiff_t> zero = {-1, 0},
                                                     ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage{chunk_sizes, max_depth, zero, offset} { }

        constexpr explicit JointMeasurementIndex(std::pair<ptrdiff_t, ptrdiff_t> zero = {-1, 0},
                                                     ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage{zero, offset} { }

    };
}