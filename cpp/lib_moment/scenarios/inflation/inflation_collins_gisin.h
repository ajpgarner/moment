/**
 * inflation_collins_gisin.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/collins_gisin.h"

namespace Moment::Inflation {

    class InflationContext;
    class InflationMatrixSystem;

    class InflationCollinsGisin : public CollinsGisin {
    public:
        const InflationContext& context;

    public:
        explicit InflationCollinsGisin(const InflationMatrixSystem& ims);

    };
}