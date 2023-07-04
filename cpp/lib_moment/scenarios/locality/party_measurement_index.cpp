/**
 * party_measurement_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "party_measurement_index.h"
#include "locality_context.h"

namespace Moment::Locality {

    PMIndex::PMIndex(const LocalityContext& context, party_name_t party, mmt_name_t mmt)
        : party{party}, mmt{mmt}, global_mmt{context.get_global_mmt_index(party, mmt)} {
    }

}