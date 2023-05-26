/**
 * export_symbol_combo.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"

#include "MatlabDataArray.hpp"

namespace Moment {
    class SymbolTable;
    class Polynomial;
}

namespace Moment::mex {
    class SymbolComboExporter : public Exporter {
    public:
        const SymbolTable& symbols;

        explicit SymbolComboExporter(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols) noexcept
                    : Exporter{engine}, symbols{symbols} { }

        /**
         * Export combo in basis form.
         * @param combo The combo to export.
         * @return Pair, first: real coefficients; second: imaginary coefficients.
         */
        std::pair<matlab::data::Array, matlab::data::Array> operator()(const Polynomial& combo) const;

        /**
         * Export combo directly as a cell array.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        matlab::data::CellArray direct(const Polynomial& combo) const;
    };
}