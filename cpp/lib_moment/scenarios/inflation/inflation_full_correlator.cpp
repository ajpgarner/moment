/**
 * inflation_full_correlator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "inflation_full_correlator.h"
#include "inflation_matrix_system.h"

#include "inflation_context.h"


namespace Moment::Inflation {

    namespace {
        FullCorrelator::TensorConstructInfo query_for_info(const InflationContext &context) {
            const auto ovc = context.observable_variant_count();
            FullCorrelator::TensorConstructInfo info;
            info.dimensions.assign(ovc, 2);
            info.operator_offset.reserve(ovc);
            for (oper_name_t i = 0; i < ovc; ++i) {
                info.operator_offset.emplace_back(i);
            }
            return info;
        }
    }

    InflationFullCorrelator::InflationFullCorrelator(const InflationMatrixSystem &system, TensorStorageType tst)
        : FullCorrelator(system.CollinsGisin(), system.polynomial_factory(),
                         query_for_info(system.InflationContext()), tst),
          context{system.InflationContext()} {

    }
}