/**
 * joint_measurement_index.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "utilities/recursive_index.h"

namespace NPATK {

    class Context;

    class JointMeasurementIndex : public MonotonicChunkRecursiveStorage<
                                            std::pair<ptrdiff_t, ptrdiff_t>, JointMeasurementIndex> {
    public:
        constexpr explicit JointMeasurementIndex(std::span<const size_t> chunk_sizes,
                                                     std::pair<ptrdiff_t, ptrdiff_t> zero = {-1, 0},
                                                     ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage{chunk_sizes, zero, offset} { }

        constexpr explicit JointMeasurementIndex(std::pair<ptrdiff_t, ptrdiff_t> zero = {-1, 0},
                                                     ptrdiff_t offset = 0)
                : MonotonicChunkRecursiveStorage{zero, offset} { }

        explicit JointMeasurementIndex(const Context& context);
    };

}