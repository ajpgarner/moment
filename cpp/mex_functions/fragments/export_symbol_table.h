/**
 * export_symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK {
    class Context;
    class SymbolTable;
}

namespace NPATK::mex {
    matlab::data::StructArray export_symbol_table_struct(matlab::engine::MATLABEngine& engine,
                                                         const Context& context, const SymbolTable& table,
                                                         size_t from_symbol = 0);

}
