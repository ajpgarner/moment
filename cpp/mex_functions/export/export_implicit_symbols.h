/**
 * export_implicit_symbols.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/linear_combo.h"
#include "scenarios/inflation/observable_variant_index.h"
#include "scenarios/locality/measurement.h"

#include <span>

namespace Moment {
    class Context;
    class MomentMatrix;
    namespace Locality {
        class LocalityImplicitSymbols;
        class LocalityOperatorFormatter;
    }

    namespace Inflation {
        class InflationImplicitSymbols;

    }

    namespace mex {
        /**
         * Export complete table of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Inflation::InflationImplicitSymbols &impliedSymbols);

        /**
         * Export one observable of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Inflation::InflationImplicitSymbols &impliedSymbols,
                               std::span<const Inflation::OVIndex> obsVarIndices);
        /**
         * Export one outcome of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Inflation::InflationImplicitSymbols &impliedSymbols,
                               std::span<const Inflation::OVOIndex> obsVarIndices);

        /**
         * Export complete table of implied symbols from locality scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Locality::LocalityOperatorFormatter& formatter,
                               const Locality::LocalityImplicitSymbols &impliedSymbols);

        /**
         * Export one measurement of implied symbols from locality scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Locality::LocalityOperatorFormatter& formatter,
                               const Locality::LocalityImplicitSymbols &impliedSymbols,
                               std::span<const Locality::PMIndex> measurementIndex);

        /**
         * Export one outcome of implied symbols from locality scenario
         */
        [[nodiscard]] matlab::data::StructArray
        export_implied_symbols(matlab::engine::MATLABEngine &engine,
                               const Locality::LocalityOperatorFormatter& formatter,
                               const Locality::LocalityImplicitSymbols &impliedSymbols,
                               std::span<const Locality::PMOIndex> outcomeIndex);
    }
}