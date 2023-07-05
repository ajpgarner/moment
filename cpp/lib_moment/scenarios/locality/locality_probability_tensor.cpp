/**
 * locality_probability_tensor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_probability_tensor.h"

#include "locality_context.h"
#include "locality_matrix_system.h"

namespace Moment::Locality {

    namespace {

        ProbabilityTensor::TensorConstructInfo make_construct_info(const LocalityContext& context) {
            ProbabilityTensor::TensorConstructInfo info;
            info.totalDimensions = context.outcomes_per_party();
            for (auto& dim : info.totalDimensions) {
                ++dim; // + identity measurement
            }
            const auto& mpp = context.measurements_per_party();
            std::copy(mpp.cbegin(), mpp.cend(), std::back_inserter(info.mmtsPerParty));
            info.outcomesPerMeasurement = context.outcomes_per_measurement();

            std::fill_n(std::back_inserter(info.fullyExplicit), info.outcomesPerMeasurement.size(), false);
            return info;
        }

    }

    LocalityProbabilityTensor::LocalityProbabilityTensor(const Moment::Locality::LocalityMatrixSystem &system,
                                                         TensorStorageType tst)
        : ProbabilityTensor{system.CollinsGisin(),
                            system.polynomial_factory(),
                            make_construct_info(system.localityContext), tst} {

        // Make GM data
        const auto& context = system.localityContext;
        for (const auto& party : context.Parties) {
            size_t dim_offset = 1; // (0 is ID)
            for (const auto& mmt : party.Measurements) {
                gmInfo.emplace_back(party.id(), dim_offset, mmt.num_outcomes);
                dim_offset += mmt.num_outcomes;
            }
        }
    }

    ProbabilityTensorRange
    LocalityProbabilityTensor::measurement_to_range(const std::span<const PMIndex> mmtIndices) const {
        ProbabilityTensorIndex lower_bounds(this->Dimensions.size(), 0);
        ProbabilityTensorIndex  upper_bounds(this->Dimensions.size(), 1);
        for (auto mmtIndex : mmtIndices) {
            if (mmtIndex.global_mmt > this->gmInfo.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmEntry = this->gmInfo[mmtIndex.global_mmt];
            if (lower_bounds[gmEntry.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmEntry.party] = gmEntry.offset;
            upper_bounds[gmEntry.party] = gmEntry.offset + gmEntry.length;
        }

        return ProbabilityTensorRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

    ProbabilityTensorRange
    LocalityProbabilityTensor::measurement_to_range(const std::span<const PMIndex> freeMeasurements,
                                                    const std::span<const PMOIndex> fixedOutcomes) const {

        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);
        for (auto mmtIndex : freeMeasurements) {
            if (mmtIndex.global_mmt > this->gmInfo.size()) {
                throw errors::BadPTError("Global measurement index out of bounds.");
            }
            const auto& gmEntry = this->gmInfo[mmtIndex.global_mmt];
            if (lower_bounds[gmEntry.party] != 0) {
                throw errors::BadPTError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmEntry.party] = gmEntry.offset;
            upper_bounds[gmEntry.party] = gmEntry.offset + gmEntry.length;
        }

        for (auto mmtIndex : fixedOutcomes) {
            if (mmtIndex.global_mmt > this->gmInfo.size()) {
                throw errors::BadPTError("Global measurement index out of bounds.");
            }
            const auto& gmEntry = this->gmInfo[mmtIndex.global_mmt];
            if (lower_bounds[gmEntry.party] != 0) {
                throw errors::BadPTError("Two measurements from same party cannot be specified.");
            }

            lower_bounds[gmEntry.party] = gmEntry.offset + mmtIndex.outcome;
            upper_bounds[gmEntry.party] = gmEntry.offset + mmtIndex.outcome + 1;
        }
        return ProbabilityTensorRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }



}