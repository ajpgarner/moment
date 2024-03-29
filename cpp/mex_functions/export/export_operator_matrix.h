/**
 * export_operator_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "exporter.h"
#include "export_operator_matrix_seq_strings.h"
#include "full_monomial_specification.h"

#include "utilities/io_parameters.h"

#include "MatlabDataArray.hpp"


namespace Moment {
    class Context;
    class SymbolicMatrix;
    class MatrixSystem;
    class MonomialMatrix;
    class PolynomialMatrix;
    class SymbolTable;

    namespace Locality {
        class LocalityMatrixSystem;
        class LocalityOperatorFormatter;
    }
}

namespace Moment::mex {

    class OperatorMatrixExporter : public ExporterWithFactory {
    public:
        const MatrixSystem& system;
        const Context& context;
        const SymbolTable& symbol_table;
        const double zero_tolerance;

        SequenceStringMatrixExporter sequence_string_exporter;


    public:
        OperatorMatrixExporter(matlab::engine::MATLABEngine& engine, const MatrixSystem& system);

        OperatorMatrixExporter(matlab::engine::MATLABEngine& engine, const Locality::LocalityMatrixSystem& system,
                               const Locality::LocalityOperatorFormatter &localityFormatter);


        /**
         * Export matrix as monomials.
         */
        [[nodiscard]] FullMonomialSpecification monomials(const MonomialMatrix& matrix) const;

        /**
         * Export matrix name.
         * @param matrix The matrix object.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::StringArray name(const SymbolicMatrix &matrix) const;

        /**
         * Export matrix as cell array of polynomials, completely defined by monomial constituents.
         */
        [[nodiscard]] matlab::data::CellArray polynomials(const SymbolicMatrix& matrix) const;

        /**
         * Export matrix properties
         */
        void properties(IOArgumentRange& output, size_t matrix_index, const SymbolicMatrix& matrix) const;

        /**
          * Outputs a matrix of operator sequences, as a matlab string matrix.
          * @param matrix The matrix object.
          * @return A matlab string array.
          */
        [[nodiscard]] matlab::data::StringArray sequence_strings(const SymbolicMatrix& matrix) const;

        /**
         * Export matrix as symbol strings.
         * @param matrix The matrix object.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::StringArray symbol_strings(const SymbolicMatrix &matrix) const;

        /**
         * Export matrix as symbol cell.
         * @param matrix The matrix object.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::CellArray symbol_cell(const SymbolicMatrix &matrix) const;

    public:
        /**
         * Return dimensions of supplied operator matrix.
         * @param matrix
         */
        [[nodiscard]] static matlab::data::ArrayDimensions matrix_dimensions(const SymbolicMatrix& matrix);

    };

}