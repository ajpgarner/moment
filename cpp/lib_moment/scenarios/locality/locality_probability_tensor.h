/**
 * locality_probability_tensor.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/probability_tensor.h"

#include "party_measurement_index.h"

namespace Moment::Locality {
    class LocalityMatrixSystem;

    class LocalityProbabilityTensor : public ProbabilityTensor {
    private:

        struct GlobalMeasurementIndex {
            /** Which dimension of tensor does this measurement correspond to */
            size_t party;
            /** How far into this dimension is this measurement? */
            size_t offset;
            /** How many operators are defined by this measurement? */
            size_t length;

            GlobalMeasurementIndex() = default;
            GlobalMeasurementIndex(size_t p, size_t o, size_t l) : party{p}, offset{o}, length{l} { }
        };

        std::vector<GlobalMeasurementIndex> gmInfo;

    public:
        explicit LocalityProbabilityTensor(const LocalityMatrixSystem& system,
                                           TensorStorageType tst = TensorStorageType::Automatic);

        ProbabilityTensorRange measurement_to_range(std::span<const PMIndex> mmtIndices) const;

        ProbabilityTensorRange measurement_to_range(std::span<const PMIndex> freeMeasurements,
                                                    std::span<const PMOIndex> fixedOutcomes) const;

    private:

    };
}