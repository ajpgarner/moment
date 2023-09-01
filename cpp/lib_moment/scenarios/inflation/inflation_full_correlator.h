/**
 * inflation_full_correlator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "probability/full_correlator.h"
#include "observable_variant_index.h"


namespace Moment::Inflation {
    class InflationMatrixSystem;
    class InflationContext;

    class InflationFullCorrelator : public FullCorrelator {
    public:
        const InflationContext& context;

    public:
        explicit InflationFullCorrelator(const InflationMatrixSystem& system,
                                         TensorStorageType tst = TensorStorageType::Automatic);


        [[nodiscard]] FullCorrelatorRange measurement_to_range(std::span<const OVIndex> mmtIndices) const;

        [[nodiscard]] FullCorrelatorRange measurement_to_range(std::span<const OVIndex> freeMeasurements,
                                                                  std::span<const OVOIndex> fixedOutcomes) const;

        [[nodiscard]] ElementView outcome_to_element(std::span<const OVOIndex> fixedOutcomes) const;
    };
}