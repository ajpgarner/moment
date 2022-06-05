/**
 * export_symbol_properties.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/symbol_tree.h"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::StructArray export_symbol_properties(matlab::engine::MATLABEngine& engine,
                                                                const SymbolTree& tree);

}
