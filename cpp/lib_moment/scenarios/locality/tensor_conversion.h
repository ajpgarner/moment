/**
 * tensor_conversion.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include "/tensor/tensor.h"

#include <span>
#include <vector>

namespace Moment::Locality {

    class LocalityContext;

    /**
     * Utility class, for converting between numerical CG Tensors and FC Tensors.
     */
    class TensorConvertor {
    public:
        using TensorType = Tensor<size_t, std::vector<size_t>, std::span<const size_t>, true>;

        /** The locality context */
        const LocalityContext& context;

        /** The expected number of tensor elements */
        const TensorType tensor_info;

        /** Constructor, sets-up tensor convertor. Raises exception if context cannot admit conversion. */
        TensorConvertor(const LocalityContext& context);

        /** Convert full correlator tensor to Collins-Gisin tensor */
        std::vector<double> full_correlator_to_collins_gisin(std::span<const double> fc_tensor) const;

        /** Convert Collins-Gisin to full correaltor tensor */
        std::vector<double> collins_gisin_to_full_correlator(std::span<const double> fc_tensor) const;

    };

}