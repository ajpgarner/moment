/**
 * export_symbol_combo.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"
#include "full_monomial_specification.h"

#include "MatlabDataArray.hpp"

#include "symbolic/polynomial.h"

#include "utilities/io_parameters.h"

#include <complex>
#include <span>
#include <utility>

namespace Moment {
    class SymbolTable;
}

namespace Moment::mex {


    class PolynomialExporter : public Exporter {
    public:
        const SymbolTable& symbols;
        const double zero_tolerance;

        explicit PolynomialExporter(matlab::engine::MATLABEngine& engine, matlab::data::ArrayFactory& factory,
                                    const SymbolTable& symbols,
                                    const double zero_tolerance) noexcept
                    : Exporter{engine, factory}, symbols{symbols}, zero_tolerance{zero_tolerance} { }

        /**
         * Export polynomial in basis form: the complex coefficients sdpvars a & b would need multiplying by.
         * @param combo The combo to export.
         * @return Pair, first: real coefficients; second: imaginary coefficients.
         */
        [[nodiscard]] std::pair<matlab::data::SparseArray<std::complex<double>>,
                                matlab::data::SparseArray<std::complex<double>>> basis(const Polynomial& poly) const;

        /**
         * Export polynomial in monolithic basis form: the complex coefficients sdpvars a & b would need multiplying by.
         * @param combo The combo to export.
         * @return Pair, first: real coefficients; second: imaginary coefficients.
         */
        [[nodiscard]] std::pair<matlab::data::SparseArray<std::complex<double>>,
                                matlab::data::SparseArray<std::complex<double>>>
        basis(std::span<const Polynomial> polys) const;

        /**
         * Export polynomial directly as a cell array.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell(const Polynomial& poly) const;

        /**
         * Export vector of polynomials as a vector of cell arrays.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell_vector(std::span<const Polynomial> poly_list) const;

        /**
         * Export polynomial as strings.
         */
        [[nodiscard]] matlab::data::MATLABString string(const Polynomial& poly, bool show_braces = true) const;

        /**
         * Export polynomial in terms of its constituent operator sequences, their hashes and weights.
         * Error if symbol not defined!
         * @param factory The array factory.
         * @param polynomial The polynomial to export.
         * @param include_symbols True to also include symbol IDs and real/imaginary basis elements.
         */
        [[nodiscard]] FullMonomialSpecification
        sequences(const Polynomial& polynomial,
                  bool include_symbols = false) const;

        /**
        * Export vector of polynomials as a vector of cell array full specifications
        * @param combo The combo to export.
        * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
        */
        [[nodiscard]] matlab::data::CellArray sequence_cell_vector(std::span<const Polynomial> poly_list,
                                                                   bool include_symbols = false) const;

    };
}