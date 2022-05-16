/**
 * substitute_elements_using_tree.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbol_tree.h"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::Array substitute_elements_using_tree(matlab::engine::MATLABEngine& engine,
                                        matlab::data::Array&& the_array,
                                        const SymbolTree& tree);
}
