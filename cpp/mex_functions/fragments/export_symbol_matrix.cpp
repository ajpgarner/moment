/**
 * exported_symbol_matrix.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "export_symbol_matrix.h"

#include "error_codes.h"
#include "operators/context.h"
#include "utilities/reporting.h"

#include "mex.hpp"

namespace NPATK::mex {

    matlab::data::Array export_symbol_matrix(matlab::engine::MATLABEngine& engine,
                                             const SquareMatrix<SymbolExpression>& inputMatrix) {
        matlab::data::ArrayFactory factory;
        matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};
        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();
        auto readIter = inputMatrix.begin();

        while ((writeIter != outputArray.end()) && (readIter != inputMatrix.end())) {
            *writeIter = readIter->as_string();
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too few input elements." );
        }
        if (readIter != inputMatrix.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too many input elements.");
        }

        return outputArray;
    }

    matlab::data::Array
    export_sequence_matrix(matlab::engine::MATLABEngine &engine,
                            const Context &context,
                            const SquareMatrix<OperatorSequence>& inputMatrix) {
        matlab::data::ArrayFactory factory;
        matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};
        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();
        auto readIter = inputMatrix.begin();

        while ((writeIter != outputArray.end()) && (readIter != inputMatrix.end())) {
            *writeIter = context.format_sequence(*readIter);
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too few input elements." );
        }
        if (readIter != inputMatrix.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix dimension mismatch: too many input elements.");
        }

        return outputArray;
    }

}
