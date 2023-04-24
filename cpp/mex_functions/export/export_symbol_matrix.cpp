/**
 * export_symbol_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_symbol_matrix.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "error_codes.h"
#include "utilities/reporting.h"


#include "mex.hpp"

namespace Moment::mex {


    matlab::data::Array SymbolMatrixExporter::operator()(const MonomialMatrix &monomialMatrix) const {

        const auto &inputMatrix = monomialMatrix.SymbolMatrix();

        matlab::data::ArrayFactory factory;
        matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};

        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();

        auto readIter = inputMatrix.ColumnMajor.begin();

        while ((writeIter != outputArray.end()) && (readIter != inputMatrix.ColumnMajor.end())) {
            *writeIter = readIter->as_string();
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix count_indices mismatch: too few input elements.");
        }
        if (readIter != inputMatrix.ColumnMajor.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix count_indices mismatch: too many input elements.");
        }

        return outputArray;
    }



    matlab::data::Array SymbolMatrixExporter::operator()(const PolynomialMatrix &matrix) const {
        throw_error(this->engine, errors::internal_error,
                    "SymbolMatrixExporter::operator()(const PolynomialMatrix &matrix) not yet implemented.");
    }

}