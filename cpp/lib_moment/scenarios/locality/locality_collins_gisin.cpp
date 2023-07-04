/**
 * locality_collins_gisin.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_collins_gisin.h"

#include "locality_context.h"
#include "locality_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment::Locality {

    namespace {
        [[nodiscard]] constexpr std::vector<size_t> make_dimensions(const std::vector<size_t>& oc) {
            std::vector<size_t> output;
            output.reserve(oc.size());
            for (auto val : oc) {
                output.push_back(val+1);
            }
            return output;
        }
    }

    LocalityCollinsGisin::LocalityCollinsGisin(const LocalityMatrixSystem &matrixSystem)
        : CollinsGisin{matrixSystem.Context(), matrixSystem.Symbols(),
                       make_dimensions(matrixSystem.localityContext.operators_per_party())},
          localityContext{matrixSystem.localityContext} {

        // Prepare global measurement -> party/measurement data
        for (const auto& party : localityContext.Parties) {
            size_t party_offset = 1; // [Offset 0 is reserved for identity operator.]
            for (const auto& mmt : party.Measurements) {
                this->gmIndex.emplace_back(static_cast<size_t>(party.id()),
                                           party_offset,
                                           static_cast<size_t>(mmt.num_operators()));
                party_offset += static_cast<size_t>(mmt.num_operators());
            }
        }

        // Prepare dimension information
        for (size_t d = 0; d < DimensionCount; ++d) {
            const auto& party = localityContext.Parties[d];
            this->dimensionInfo[d].op_ids.reserve(this->Dimensions[d]);
            this->dimensionInfo[d].op_ids.emplace_back(-1); // index 0 is always no-op.
            const auto party_ops = party.operators();
            std::copy(party_ops.begin(), party_ops.end(), std::back_inserter(this->dimensionInfo[d].op_ids));
        }

        // Build array in column-major format, for quick export to matlab.
        if (this->StorageType == TensorStorageType::Explicit) {
            for (const auto &cgIndex: MultiDimensionalIndexRange<true>{Dimensions}) {
                this->data.emplace_back(*this, cgIndex);
            }

            // Try to find initial symbols
            this->do_initial_symbol_search();
        }
    }


    CollinsGisin::CollinsGisinRange
    LocalityCollinsGisin::measurement_to_range(const std::span<const PMIndex> mmtIndices) const {
        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);
        for (auto mmtIndex : mmtIndices) {
            if (mmtIndex.global_mmt > this->gmIndex.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmInfo = this->gmIndex[mmtIndex.global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmInfo.party] = gmInfo.offset;
            upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }

    CollinsGisin::CollinsGisinRange
    LocalityCollinsGisin::measurement_to_range(const std::span<const PMIndex> mmtIndices,
                                               const std::span<const PMOIndex> fixedOutcomes) const {
        CollinsGisinIndex lower_bounds(this->Dimensions.size(), 0);
        CollinsGisinIndex upper_bounds(this->Dimensions.size(), 1);
        for (auto mmtIndex : mmtIndices) {
            if (mmtIndex.global_mmt > this->gmIndex.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmInfo = this->gmIndex[mmtIndex.global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }
            lower_bounds[gmInfo.party] = gmInfo.offset;
            upper_bounds[gmInfo.party] = gmInfo.offset + gmInfo.length;
        }

        for (auto mmtIndex : fixedOutcomes) {
            if (mmtIndex.global_mmt > this->gmIndex.size()) {
                throw errors::BadCGError("Global measurement index out of bounds.");
            }
            const auto& gmInfo = this->gmIndex[mmtIndex.global_mmt];
            if (lower_bounds[gmInfo.party] != 0) {
                throw errors::BadCGError("Two measurements from same party cannot be specified.");
            }

            lower_bounds[gmInfo.party] = gmInfo.offset + mmtIndex.outcome;
            upper_bounds[gmInfo.party] = gmInfo.offset + mmtIndex.outcome + 1;
        }
        return CollinsGisinRange{*this, std::move(lower_bounds), std::move(upper_bounds)};
    }
}
