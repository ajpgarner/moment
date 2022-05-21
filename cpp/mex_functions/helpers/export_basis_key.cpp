/**
 * export_basis_key.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "export_basis_key.h"
#include "MatlabDataArray/ArrayFactory.hpp"


namespace NPATK::mex {

    matlab::data::TypedArray<int32_t>
    export_basis_key(matlab::engine::MATLABEngine &engine, const IndexMatrixProperties &imp) {
        matlab::data::ArrayFactory factory{};
        matlab::data::ArrayDimensions dims{imp.BasisMap().size(),
                                           (imp.Type() == IndexMatrixProperties::BasisType::Hermitian) ? 3U : 2U};
        matlab::data::TypedArray<int32_t> output = factory.createArray<int32_t>(dims);

        if (imp.Type() == IndexMatrixProperties::BasisType::Hermitian) {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisMap()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                output[index][2] = static_cast<int32_t>(re_im_pair.second) + 1;
                ++index;
            }
        } else {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisMap()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                ++index;
            }
        }


        return output;
    }
}