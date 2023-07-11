/**
 * export_sequence_matrix.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "dictionary/operator_sequence.h"
#include "symbolic/monomial.h"
#include "utilities/square_matrix.h"

#include "exporter.h"

namespace Moment {
    class OperatorMatrix;
    class MonomialMatrix;
    class PolynomialMatrix;
    class MatrixSystem;

    namespace Locality {
        class LocalityContext;
        class LocalityOperatorFormatter;
        class LocalityMatrixSystem;
    }

    namespace Inflation {
        class InflationMatrixSystem;
    }
}

namespace Moment::mex {


    class SequenceStringMatrixExporter : public Exporter {

        const MatrixSystem& system;
        const Locality::LocalityOperatorFormatter* localityFormatterPtr = nullptr;
        const Locality::LocalityContext* localityContextPtr = nullptr;
        const Inflation::InflationMatrixSystem* imsPtr = nullptr;

    public:
        explicit SequenceStringMatrixExporter(matlab::engine::MATLABEngine& engine,
                                              matlab::data::ArrayFactory& factory,
                                              const MatrixSystem& system) noexcept;

        explicit SequenceStringMatrixExporter(matlab::engine::MATLABEngine& engine,
                                              matlab::data::ArrayFactory& factory,
                                              const Locality::LocalityMatrixSystem& system,
                                              const Locality::LocalityOperatorFormatter& localityFormatter) noexcept;

        /**
         * Outputs a matrix of operator sequences, as a matlab string matrix.
         * @param matrix The matrix object.
         * @param system The system the matrix belongs to.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::StringArray operator()(const MonomialMatrix& matrix) const;

        /**
         * Outputs a matrix of operator sequences, as a matlab string matrix.
         * @param matrix The matrix object.
         * @param system The system the matrix belongs to.
         * @return A matlab string array.
         */
        [[nodiscard]] matlab::data::StringArray operator()(const PolynomialMatrix& matrix) const;
    };


}
