/**
 * inflation_collins_gisin.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/collins_gisin.h"

#include "observable_variant_index.h"

namespace Moment::Inflation {

    class InflationContext;
    class InflationMatrixSystem;

    class InflationCollinsGisin : public CollinsGisin {
    public:
        const InflationContext& inflationContext;

    public:
        explicit InflationCollinsGisin(const InflationMatrixSystem& ims);

        using CollinsGisin::measurement_to_range;

        /**
         * Splice all operators belonging to a supplied set of measurements indices.
         * @return Spliced range.
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const OVIndex> mmtIndices) const;

        /**
         * Splice all operators belonging to a supplied set of measurements indices.
         * @return Spliced range.
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const OVIndex> freeMeasurements,
                                                             std::span<const OVOIndex> fixedOutcomes) const;

    protected:
        /**
         * Overloaded symbol look up, also considers canonical observable variants.
         */
        const class Symbol* try_find_symbol(const OperatorSequence &seq) const noexcept override;
    };
}