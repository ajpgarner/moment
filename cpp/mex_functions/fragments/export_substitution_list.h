/**
 * export_substitution_list.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"

#include "MatlabDataArray.hpp"
#include "symbol_tree.h"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::Array export_substitution_list(matlab::engine::MATLABEngine& engine,
                                                       const SymbolTree& tree);
}
