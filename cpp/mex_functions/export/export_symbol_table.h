/**
 * export_symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "integer_types.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class MatrixSystem;
    class UniqueSequence;
}

namespace Moment::mex {
    class EnvironmentalVariables;

    matlab::data::StructArray export_symbol_table_row(matlab::engine::MATLABEngine& engine,
                                                      const EnvironmentalVariables& env,
                                                      const MatrixSystem& system, const UniqueSequence& symbol);

    matlab::data::StructArray export_symbol_table_struct(matlab::engine::MATLABEngine& engine,
                                                         const EnvironmentalVariables& env,
                                                         const MatrixSystem& system,
                                                         size_t from_symbol = 0);

}
