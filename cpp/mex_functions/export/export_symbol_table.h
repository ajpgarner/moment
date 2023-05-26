/**
 * export_symbol_table.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "integer_types.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class MatrixSystem;
    class Symbol;
}

namespace Moment::mex {
    class EnvironmentalVariables;

    matlab::data::StructArray export_symbol_table_row(matlab::engine::MATLABEngine& engine,
                                                      const EnvironmentalVariables& env,
                                                      const MatrixSystem& system, const Symbol& symbol);

    matlab::data::StructArray export_symbol_table_struct(matlab::engine::MATLABEngine& engine,
                                                         const EnvironmentalVariables& env,
                                                         const MatrixSystem& system,
                                                         size_t from_symbol = 0);

}
