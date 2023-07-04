/**
 * locality_collins_gisin.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "scenarios/collins_gisin.h"
#include "party_measurement_index.h"


namespace Moment::Locality {

    class LocalityContext;
    class LocalityMatrixSystem;

    class LocalityCollinsGisin : public CollinsGisin {
    public:
        const LocalityContext& localityContext;

    public:
        explicit LocalityCollinsGisin(const LocalityMatrixSystem& lms);

        using CollinsGisin::measurement_to_range;

        /**
         * Splice all operators belonging to a supplied set of measurements indices.
         * @return Spliced range.
         * @throws BadCGError If index is invalid.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const PMIndex> mmtIndices) const;

        /**
         * Splice all operators belonging to a supplied set of measurements indices.
         * @return Spliced range.
         * @throws BadCGError If index is invalid, or if free and fixed measurements overlap.
         */
        [[nodiscard]] CollinsGisinRange measurement_to_range(std::span<const PMIndex> freeMeasurements,
                                                             std::span<const PMOIndex> fixedOutcomes) const;

    };
}