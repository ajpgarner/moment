/**
 * locality_full_correlator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "locality_full_correlator.h"

#include "locality_matrix_system.h"
#include "locality_context.h"

namespace Moment::Locality {
    namespace {
        LocalityFullCorrelator::TensorConstructInfo query_for_info(const LocalityContext &context) {
            LocalityFullCorrelator::TensorConstructInfo info;

            info.dimensions.reserve(context.Parties.size());
            info.operator_offset.reserve(context.Parties.size());
            for (const auto& party : context.Parties) {
                info.dimensions.emplace_back(party.Measurements.size() + 1);
                info.operator_offset.emplace_back(party.global_offset());
            }
            return info;
        }
    }

    LocalityFullCorrelator::LocalityFullCorrelator(const LocalityMatrixSystem &system, TensorStorageType tst)
            : FullCorrelator(system.CollinsGisin(), system.polynomial_factory(),
                             query_for_info(system.localityContext), tst),
              context{system.localityContext} {

    }

    LocalityFullCorrelator::ElementView
    LocalityFullCorrelator::mmt_to_element(std::span<const PMIndex> mmtIndices) const {
        AutoStorageIndex index(this->Dimensions.size(), 0);

        for (auto mmtIndex : mmtIndices) {
            if (mmtIndex.party >= this->DimensionCount) {
                throw Moment::errors::BadFCError("Party index out of bounds.");
            }
            const auto& partyInfo = this->context.Parties[mmtIndex.party];

            if (index[mmtIndex.party] != 0) {
                throw Moment::errors::BadFCError("Two measurements from same party cannot be specified.");
            }
            if (mmtIndex.mmt >= partyInfo.Measurements.size()) {
                throw Moment::errors::BadFCError("Measurement index out of bounds.");
            }
            index[mmtIndex.party] = mmtIndex.mmt + 1;
        }

        return this->elem_no_checks(index);
    }
}