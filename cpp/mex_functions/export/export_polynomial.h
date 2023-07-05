/**
 * export_symbol_combo.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"

#include "MatlabDataArray.hpp"

#include "utilities/io_parameters.h"

#include <complex>
#include <utility>

namespace Moment {
    class SymbolTable;
    class Polynomial;
}

namespace Moment::mex {

    struct FullPolynomialSpecification {
        const std::vector<size_t> dimensions;
        const bool has_symbol_info;

        matlab::data::CellArray operators;
        matlab::data::TypedArray<std::complex<double>> coefficients;
        matlab::data::TypedArray<uint64_t> hashes;
        matlab::data::TypedArray<int64_t> symbol_ids;
        matlab::data::TypedArray<bool> is_conjugated;
        matlab::data::TypedArray<int64_t> real_basis_elems;
        matlab::data::TypedArray<int64_t> im_basis_elems;

    public:
        FullPolynomialSpecification(matlab::data::ArrayFactory& factory, size_t length, bool include_symbol_info);

        FullPolynomialSpecification(const FullPolynomialSpecification& rhs) = delete;

        FullPolynomialSpecification(FullPolynomialSpecification&& rhs) noexcept = default;

        /** Export polynomial to output range. */
        void move_to_output(IOArgumentRange& output) noexcept;

        /** Export polynomial as a cell array of constituent parts. */
        matlab::data::CellArray move_to_cell(matlab::data::ArrayFactory& factory);
    };

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
         * Export polynomial directly as a cell array.
         * @param combo The combo to export.
         * @return Cell array of cell pairs/triplets {{id, factor, [true, if conjugated]}}
         */
        [[nodiscard]] matlab::data::CellArray direct(const Polynomial& combo) const;

        /**
         * Export polynomial in terms of its constituent operator sequences, their hashes and weights.
         * Error if symbol not defined!
         * @param factory The array factory.
         * @param polynomial The polynomial to export.
         * @param include_symbols True to also include symbol IDs and real/imaginary basis elements.
         */
        [[nodiscard]] FullPolynomialSpecification
        sequences(matlab::data::ArrayFactory& factory,
                  const Polynomial& polynomial,
                  bool include_symbols = false) const;

        /**
         * Export polynomial in terms of its constituent operator sequences, their hashes and weights.
         * Error if symbol not defined!
         * @param polynomial The polynomial to export.
         * @param include_symbols True to also include symbol IDs and real/imaginary basis elements.
         */
        [[nodiscard]] inline FullPolynomialSpecification
        sequences(const Polynomial& polynomial, bool include_symbols = false) const {
            matlab::data::ArrayFactory factory;
            return sequences(factory, polynomial, include_symbols);
        }
    };
}