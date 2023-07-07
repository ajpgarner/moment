/**
 * inflation_probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_probability_tensor.h"

#include "inflation_context.h"
#include "inflation_matrix_system.h"

namespace Moment::Inflation {
    namespace {
        ProbabilityTensor::TensorConstructInfo make_construct_info(const InflationContext &context) {
            ProbabilityTensor::TensorConstructInfo info;
            for (const auto &observable: context.Observables()) {
                if (observable.projective()) {
                    std::fill_n(std::back_inserter(info.totalDimensions), observable.variant_count,
                                observable.outcomes + 1); // ID, then Measurement
                    std::fill_n(std::back_inserter(info.outcomesPerMeasurement), observable.variant_count,
                                observable.outcomes);
                    std::fill_n(std::back_inserter(info.fullyExplicit), observable.variant_count, false);
                } else {
                    std::fill_n(std::back_inserter(info.totalDimensions), observable.variant_count,
                                2); // ID, operator.
                    std::fill_n(std::back_inserter(info.outcomesPerMeasurement), observable.variant_count, 1);
                    std::fill_n(std::back_inserter(info.fullyExplicit), observable.variant_count, true);
                }
            }
            std::fill_n(std::back_inserter(info.mmtsPerParty), context.observable_variant_count(), 1);
            return info;
        }
    }


    InflationProbabilityTensor::InflationProbabilityTensor(const InflationMatrixSystem& system)
        : ProbabilityTensor(system.CollinsGisin(), system.polynomial_factory(),
                            make_construct_info(system.InflationContext())),
          context{system.InflationContext()} {
    }


    ProbabilityTensorRange
    InflationProbabilityTensor::measurement_to_range(const std::span<const OVIndex> mmtIndices) const {
        ProbabilityTensorIndex lower_bounds(this->Dimensions.size(), 0);
        ProbabilityTensorIndex upper_bounds(this->Dimensions.size(), 1);
        for (const auto& mmtIndex : mmtIndices) {

            const size_t global_index = this->context.obs_variant_to_index(mmtIndex);
            if (global_index > this->DimensionCount) {
                throw Moment::errors::BadPTError("Global measurement index out of bounds.");
            }

            if (lower_bounds[global_index] != 0) {
                throw Moment::errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[global_index] = 1;
            upper_bounds[global_index] = this->Dimensions[global_index];
        }

        return ProbabilityTensorRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

    ProbabilityTensorRange
    InflationProbabilityTensor::measurement_to_range(const std::span<const OVIndex> freeMeasurements,
                                                    const std::span<const OVOIndex> fixedOutcomes) const {
        ProbabilityTensorIndex lower_bounds(this->Dimensions.size(), 0);
        ProbabilityTensorIndex upper_bounds(this->Dimensions.size(), 1);
        for (const auto& mmtIndex : freeMeasurements) {
            const size_t global_index = this->context.obs_variant_to_index(mmtIndex);
            if (global_index > this->DimensionCount) {
                throw Moment::errors::BadPTError("Global measurement index out of bounds.");
            }
            if (lower_bounds[global_index] != 0) {
                throw Moment::errors::BadPTError("Two measurements of the same observable cannot be specified.");
            }
            lower_bounds[global_index] = 1;
            upper_bounds[global_index] = this->Dimensions[global_index];
        }

        for (const auto& mmtIndex : fixedOutcomes) {
            const size_t global_index = this->context.obs_variant_to_index(mmtIndex.observable_variant);
            if (global_index > this->DimensionCount) {
                throw Moment::errors::BadPTError("Global measurement index out of bounds.");
            }
            if (lower_bounds[global_index] != 0) {
                throw Moment::errors::BadPTError("Two measurements of the same observable cannot be specified.");
            }

            lower_bounds[global_index] = 1 + mmtIndex.outcome;
            upper_bounds[global_index] = 2 + mmtIndex.outcome;
        }
        return ProbabilityTensorRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }


    ProbabilityTensor::ElementView
    InflationProbabilityTensor::outcome_to_element(std::span<const OVOIndex> fixedOutcomes) const {
        ProbabilityTensorIndex index(this->Dimensions.size(), 0);
        for (const auto& mmtIndex : fixedOutcomes) {
            const size_t global_index = this->context.obs_variant_to_index(mmtIndex.observable_variant);
            if (global_index > this->DimensionCount) {
                throw Moment::errors::BadPTError("Global measurement index out of bounds.");
            }
            if (index[global_index] != 0) {
                throw Moment::errors::BadPTError("Two measurements of the same observable cannot be specified.");
            }

            index[global_index] = 1 + mmtIndex.outcome;
        }
        return this->elem_no_checks(index);
    }

}