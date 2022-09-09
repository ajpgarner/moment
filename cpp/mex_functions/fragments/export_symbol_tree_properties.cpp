/**
 * export_symbol_tree_properties.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "export_symbol_tree_properties.h"


namespace NPATK::mex {
    matlab::data::StructArray export_symbol_properties(matlab::engine::MATLABEngine& engine,
                                                               const SymbolTree& tree) {
        assert(tree.ready());
        matlab::data::ArrayFactory factory{};
        matlab::data::ArrayDimensions dims{1, tree.count_nodes()-1};
        auto output = factory.createStructArray(dims, {"id", "has_real", "has_im"});
        for (size_t i = 1, iMax = tree.count_nodes(); i < iMax; ++i) { // Skip 0
            const auto& node = tree[i];

            output[i-1]["id"] = factory.createScalar(static_cast<double>(tree[i].id));
            output[i-1]["has_real"] = factory.createScalar<bool>(!tree[i].real_is_zero);
            output[i-1]["has_im"] = factory.createScalar<bool>(!tree[i].im_is_zero);
        }
        return output;
    }
}
