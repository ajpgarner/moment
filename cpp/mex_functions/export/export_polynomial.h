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
    class RawPolynomial;
}

namespace Moment::mex {


    class PolynomialExporter : public Exporter {
    public:
        const Context& context;
        const SymbolTable& symbols;
        const double zero_tolerance;

        explicit PolynomialExporter(matlab::engine::MATLABEngine& engine, matlab::data::ArrayFactory& factory,
                                    const Context& context, const SymbolTable& symbols,
                                    const double zero_tolerance) noexcept
                    : Exporter{engine, factory}, context{context}, symbols{symbols}, zero_tolerance{zero_tolerance} { }

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
         * @param poly The polynomial to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell(const Polynomial& poly) const;

        /**
         * Export monomial as a cell array.
         * @param mono The monomial to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell(const Monomial& mono) const;

        /**
         * Export vector of polynomials as a vector of cell arrays.
         * @param poly_list The polynomials to export.
         * @param shape The shape of the symbol cell.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell_vector(const std::span<const Polynomial> poly_list,
                                                                 matlab::data::ArrayDimensions shape) const;


        /**
         * Export vector of polynomials as a vector of cell arrays.
         * @param poly_list The polynomials to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell_vector(const std::span<const Polynomial> poly_list) const {
            return this->symbol_cell_vector(poly_list, matlab::data::ArrayDimensions{poly_list.size(), 1});
        }

        /**
         * Export vector of monomials as a vector of cell arrays.
         * @param combo The monomials to export.
         * @param shape The shape of the symbol cell.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell_vector(const std::span<const Monomial> mono_list,
                                                                 matlab::data::ArrayDimensions shape) const;

        /**
         * Export vector of monomials as a vector of cell arrays.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell_vector(const std::span<const Monomial> mono_list) const {
            return this->symbol_cell_vector(mono_list, matlab::data::ArrayDimensions{mono_list.size(), 1});
        }

        /**
         * Export polynomial as strings.
         */
        [[nodiscard]] matlab::data::MATLABString string(const Polynomial& poly, bool show_braces = true) const;

        /**
         * Export polynomial in terms of its constituent operator sequences, their hashes and weights.
         * Error if symbol not defined!
         * @param polynomial The polynomial to export.
         * @param include_symbols True to also include symbol IDs and real/imaginary basis elements.
         */
        [[nodiscard]] FullMonomialSpecification
        sequences(const Polynomial& polynomial,
                  bool include_symbols = false) const;

        /**
         * Export raw polynomial in terms of its constituent operator sequences, their hashes and weights.
         * @param polynomial The polynomial to export.
         */
        [[nodiscard]] FullMonomialSpecification
        sequences(const RawPolynomial& polynomial) const;

        /**
         * Export vector of polynomials as a vector of cell array full specifications
         * @param poly_list The list of polynomials to export.
         */
        [[nodiscard]] matlab::data::CellArray sequence_cell_vector(std::span<const Polynomial> poly_list,
                                                                   const std::vector<size_t>& shape,
                                                                   bool include_symbols = false) const;

       /**
        * Export vector of polynomials as a vector of cell array specifications
        * @param poly_list The list of polynomials to export.
        */
        [[nodiscard]] matlab::data::CellArray sequence_cell_vector(std::span<const RawPolynomial> poly_list,
                                                                   const std::vector<size_t>& shape) const;

        /**
        * Export vector of polynomials as a cell array full monomial specification.
        * @param poly The list of polynomials to export. Each must be monomial (0 or 1 elements).
        * @return Full monomial specification, as matlab array.
        */
        [[nodiscard]] FullMonomialSpecification monomial_sequence_cell_vector(std::span<const Polynomial> poly_list,
                                                                              const std::vector<size_t>& shape,
                                                                              bool include_symbols = false) const;

    };
}