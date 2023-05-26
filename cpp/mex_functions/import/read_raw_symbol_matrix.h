/**
 * read_raw_symbol_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once


#include "utilities/square_matrix.h"
#include "symbolic/monomial.h"
#include "integer_types.h"

#include "MatlabDataArray.hpp"

#include <memory>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    std::unique_ptr<SquareMatrix<Monomial>>
    read_raw_symbol_matrix(matlab::engine::MATLABEngine &matlabEngine,
                           const matlab::data::Array& input);

}