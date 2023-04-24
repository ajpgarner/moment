/**
 * export_sequence_matrix.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "scenarios/operator_sequence.h"
#include "symbolic/symbol_expression.h"
#include "utilities/square_matrix.h"

#include "exporter.h"

namespace Moment {
    class OperatorMatrix;
    class MonomialMatrix;
    class PolynomialMatrix;
    class MatrixSystem;

    namespace Locality {
        class LocalityOperatorFormatter;
    }

    namespace Inflation {
        class InflationContext;
        class FactorTable;
    }
}

namespace Moment::mex {


    class SequenceMatrixExporter : public Exporter {

    public:
        explicit SequenceMatrixExporter(matlab::engine::MATLABEngine& engine) noexcept
            : Exporter{engine} { }

        /**
         * Outputs a matrix of operator sequences, as a matlab string matrix.
         * @param matrix The matrix object.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::Array operator()(const OperatorMatrix& op_matrix) const;

        /**
          * Outputs a matrix of operator sequences, as a matlab string matrix, using locality formatter
          * @param matrix The matrix object.
          * @param formatter Object for formatting display of locality operators.
          * @return A matlab string array.
          */
        [[nodiscard]] matlab::data::Array operator()(const MonomialMatrix& matrix,
                                                     const Locality::LocalityOperatorFormatter& formatter) const;

        /**
         * Outputs a matrix of operator sequences, as a matlab string matrix.
         * @param matrix The matrix object.
         * @param system The system the matrix belongs to.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::Array operator()(const MonomialMatrix& matrix, const MatrixSystem& system) const;

        /**
         * Outputs a matrix of operator sequences, as a matlab string matrix.
         * @param matrix The matrix object.
         * @param system The system the matrix belongs to.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::Array operator()(const PolynomialMatrix& matrix, const MatrixSystem& system) const;

    private:
        [[nodiscard]] matlab::data::Array export_inferred(const MonomialMatrix& matrix) const;

        [[nodiscard]] matlab::data::Array export_direct(const OperatorMatrix& opMatrix) const;

        [[nodiscard]] matlab::data::Array export_factored(const Inflation::InflationContext& context,
                                                          const Inflation::FactorTable& factors,
                                                          const MonomialMatrix& inputMatrix) const;
    };


}
