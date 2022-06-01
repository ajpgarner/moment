/**
 * make_symmetric_using_tree.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbol_tree.h"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::Array make_symmetric_using_tree(matlab::engine::MATLABEngine& engine,
                                                                const matlab::data::Array& the_array,
                                                                const SymbolTree& tree, bool sparse_output);

    [[nodiscard]] matlab::data::TypedArray<matlab::data::MATLABString> make_hermitian_using_tree(
                                        matlab::engine::MATLABEngine& engine,
                                        const matlab::data::TypedArray<matlab::data::MATLABString>& the_array,
                                        const SymbolTree& tree);
}
