/**
 * inflation_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_collins_gisin.h"

#include "inflation_matrix_system.h"
#include "inflation_context.h"

#include "symbolic/symbol_table.h"

#include "utilities/multi_dimensional_index_iterator.h"


namespace Moment::Inflation {

    namespace {
        [[nodiscard]] inline std::vector<size_t> make_dimensions(const InflationContext &context) {

            const size_t observable_count = context.observable_variant_count();
            std::vector<size_t> output;
            output.reserve(observable_count);

            for (const auto &observable: context.Observables()) {
                // (outcomes - 1) + 1; for operators, but also with identity.
                if (observable.projective()) {
                    std::fill_n(std::back_inserter(output), observable.variant_count, observable.outcomes);
                } else {
                    // Non-projective operator is I or Op
                    std::fill_n(std::back_inserter(output), observable.variant_count, 2);
                }
            }
            assert(output.size() == observable_count);

            return output;
        }
    }

    InflationCollinsGisin::InflationCollinsGisin(const InflationMatrixSystem &matrixSystem)
            : CollinsGisin{matrixSystem.Context(), matrixSystem.Symbols(),
                           make_dimensions(matrixSystem.InflationContext())},
              inflationContext{matrixSystem.InflationContext()} {

        // Prepare global measurement -> party/measurement data.
        this->gmIndex.reserve(this->DimensionCount);
        size_t d = 0;
        for (const auto &observable: inflationContext.Observables()) {
            if (observable.projective()) {
                const size_t op_num = observable.outcomes - 1;
                for (const auto &variant: observable.variants) {

                    this->dimensionInfo[d].op_ids.reserve(this->Dimensions[d]);
                    this->dimensionInfo[d].op_ids.emplace_back(-1); // index 0 is always no-op.

                    for (size_t outcome = 0; outcome < op_num; ++outcome) {
                        this->dimensionInfo[d].op_ids.emplace_back(variant.operator_offset + outcome);
                    }

                    this->gmIndex.emplace_back(d, 1, op_num);
                    ++d;
                }
            } else {
                for (const auto &variant: observable.variants) {
                    this->dimensionInfo[d].op_ids.reserve(this->Dimensions[d]);
                    this->dimensionInfo[d].op_ids.emplace_back(-1); // index 0 is always no-op.
                    this->dimensionInfo[d].op_ids.emplace_back(variant.operator_offset);
                    this->gmIndex.emplace_back(d, 1, 1);
                    ++d;
                }
            }
        }

        if (this->StorageType == TensorStorageType::Explicit) {
            // Make from tensor indices
            for (const auto &cgIndex: MultiDimensionalIndexRange<true>{Dimensions}) {
                this->data.emplace_back(*this, cgIndex);
            }

            // Try to find symbols
            this->do_initial_symbol_search();
        }
    }


    CollinsGisinRange
    InflationCollinsGisin::measurement_to_range(const std::span<const OVIndex> mmtIndices) const {
        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);

        for (auto mmtIndex: mmtIndices) {
            const size_t global_mmt = this->inflationContext.obs_variant_to_index(mmtIndex);
            if (global_mmt > this->gmIndex.size()) {
                throw Moment::errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto &gmInfo = this->gmIndex[global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw Moment::errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmInfo.party] = gmInfo.offset;
            upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

    CollinsGisinRange
    InflationCollinsGisin::measurement_to_range(const std::span<const OVIndex> mmtIndices,
                                                const std::span<const OVOIndex> fixedOutcomes) const {
        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);
        for (auto mmtIndex: mmtIndices) {
            const size_t global_mmt = this->inflationContext.obs_variant_to_index(mmtIndex);
            if (global_mmt > this->gmIndex.size()) {
                throw Moment::errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto &gmInfo = this->gmIndex[global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw Moment::errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmInfo.party] = gmInfo.offset;
            upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
        }

        for (auto mmtIndex: fixedOutcomes) {
            const size_t global_mmt = this->inflationContext.obs_variant_to_index(mmtIndex.observable_variant);
            if (global_mmt > this->gmIndex.size()) {
                throw Moment::errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto &gmInfo = this->gmIndex[global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw Moment::errors::BadCGError("Two measurements from same party cannot be specified.");
            }

            lower_bounds[gmInfo.party] = gmInfo.offset + mmtIndex.outcome;
            upper_bounds[gmInfo.party] = gmInfo.offset + mmtIndex.outcome + 1;
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

}
