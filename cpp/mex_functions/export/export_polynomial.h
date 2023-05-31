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
    class PolynomialExporter : public Exporter {
    public:
        const SymbolTable& symbols;

        explicit PolynomialExporter(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols) noexcept
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

        /**
         * Export combo as a cell array, translating all symbols into operator sequences.
         * Error if symbol not defined!
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{[op sequence array], factor}}
         */
        matlab::data::CellArray sequences(const Polynomial& combo) const;
    };
}