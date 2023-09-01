/**
 * locality_full_correlator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "probability/full_correlator.h"

#include "party_measurement_index.h"

namespace Moment::Locality {
    class LocalityMatrixSystem;

    class LocalityFullCorrelator : public FullCorrelator {
    public:
        const LocalityContext& context;

    public:
        explicit LocalityFullCorrelator(const LocalityMatrixSystem& system,
                                        TensorStorageType tst = TensorStorageType::Automatic);

        virtual ~LocalityFullCorrelator() noexcept = default;

        /**
         * Get correlator element from party/measurement indices.
         */
        [[nodiscard]] ElementView mmt_to_element(std::span<const PMIndex> mmtIndices) const;
    };
}