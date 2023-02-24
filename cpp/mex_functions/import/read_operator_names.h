/**
 * read_operator_names.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "integer_types.h"

#include "scenarios/algebraic/name_table.h"

#include <memory>

namespace Moment::mex {

    /**
     * Parse matlab array into an Algebraic::NameTable.
     * @param matlabEngine Handle to MATLAB.
     * @param input The array to attempt to parse.
     * @param paramName The name of the input argument, used in error messages.
     * @return Name table with parsed names.
     */
    [[nodiscard]] std::unique_ptr<Algebraic::NameTable>
    read_name_table(matlab::engine::MATLABEngine& matlabEngine, matlab::data::Array& input,
                    const std::string& paramName);

}