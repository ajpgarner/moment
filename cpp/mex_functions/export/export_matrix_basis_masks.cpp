/**
 * export_matrix_basis_masks.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_matrix_basis_masks.h"

#include "matrix/matrix_properties.h"
#include "matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"

#include "error_codes.h"

#include "mex.hpp"

namespace Moment::mex {

    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<uint64_t>>
    export_basis_lists(matlab::engine::MATLABEngine &engine,
                       const SymbolTable& symbol_table, const MatrixProperties &smp) {
        // Prepare output lists
        matlab::data::ArrayFactory factory;
        auto output = std::make_pair(
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, smp.RealSymbols().size()}),
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, smp.ImaginarySymbols().size()})
        );

        auto re_write_iter = output.first.begin();
        auto im_write_iter = output.second.begin();

        for (auto [symbol_id, basis_elems] : smp.BasisKey()) {
            if (basis_elems.first >= 0) {
                assert(re_write_iter != output.first.end());
                *re_write_iter = static_cast<int32_t>(basis_elems.first) + 1; // + 1 for matlab indexing
                ++re_write_iter;
            }
            if (basis_elems.second >= 0) {
                assert(im_write_iter != output.second.end());
                *im_write_iter = static_cast<int32_t>(basis_elems.second) + 1; // + 1 for matlab indexing
                ++im_write_iter;
            }
        }

        return output;
    }


    std::pair<matlab::data::TypedArray<bool>, matlab::data::TypedArray<bool>>
    export_basis_masks(matlab::engine::MATLABEngine& engine,
                       const SymbolTable& symbol_table,
                       const MatrixProperties& smp) {

        // Prepare masks
        matlab::data::ArrayFactory factory;

        // Create arrays, make them all false
        const auto real_symbol_count = symbol_table.RealSymbolIds().size();
        const auto imaginary_symbol_count = symbol_table.ImaginarySymbolIds().size();

        auto output = std::make_pair(
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, real_symbol_count}),
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, imaginary_symbol_count})
        );
        std::fill(output.first.begin(), output.first.end(), false);
        std::fill(output.second.begin(), output.second.end(), false);

        for (auto [symbol_id, basis_elems] : smp.BasisKey()) {
            if (basis_elems.first >= 0) {
                assert(basis_elems.first < real_symbol_count);
                output.first[basis_elems.first] = true;
            }
            if (basis_elems.second >= 0) {
                assert(basis_elems.second < imaginary_symbol_count);
                output.second[basis_elems.second] = true;
            }
        }

        return output;
    }

    matlab::data::TypedArray<int32_t>
    export_basis_key(matlab::engine::MATLABEngine &engine, const MatrixProperties &imp) {
        matlab::data::ArrayFactory factory{};
        matlab::data::ArrayDimensions dims{imp.BasisKey().size(),
                                           (imp.Type() == MatrixType::Hermitian) ? 3U : 2U};
        matlab::data::TypedArray<int32_t> output = factory.createArray<int32_t>(dims);

        if (imp.Type() == MatrixType::Hermitian) {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                output[index][2] = static_cast<int32_t>(re_im_pair.second) + 1;
                ++index;
            }
        } else {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: imp.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                ++index;
            }
        }
        return output;
    }

}