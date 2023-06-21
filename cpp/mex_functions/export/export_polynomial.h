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
        const double zero_tolerance;

        explicit PolynomialExporter(matlab::engine::MATLABEngine& engine, const SymbolTable& symbols,
                                    const double zero_tolerance) noexcept
                    : Exporter{engine}, symbols{symbols}, zero_tolerance{zero_tolerance} { }

        /**
         * Export combo in basis form: the complex coefficients sdpvars a & b would need multiplying by.
         * @param combo The combo to export.
         * @return Pair, first: real coefficients; second: imaginary coefficients.
         */
        [[nodiscard]] std::pair<matlab::data::SparseArray<std::complex<double>>,
                                matlab::data::SparseArray<std::complex<double>>> basis(const Polynomial& combo) const;

        /**
         * Export combo directly as a cell array.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray direct(const Polynomial& combo) const;

        /**
         * Export combo as a cell array, translating all symbols into operator sequences.
         * Error if symbol not defined!
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{[op sequence array], factor}}
         */
        [[nodiscard]] matlab::data::CellArray sequences(const Polynomial& combo) const;
    };
}