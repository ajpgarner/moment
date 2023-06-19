/**
 * export_matrix_basis_masks.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_matrix_basis_masks.h"

#include "matrix/matrix.h"
#include "matrix/operator_matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"
#include "symbolic/symbol_table.h"

#include "error_codes.h"

#include "mex.hpp"

namespace Moment::mex {

    matlab::data::TypedArray<int32_t> BasisKeyExporter::basis_key(const Matrix &matrix) {

        matlab::data::ArrayFactory factory{};
        matlab::data::ArrayDimensions dims{matrix.BasisKey().size(),
                                           (matrix.HasComplexBasis()) ? 3U : 2U};
        matlab::data::TypedArray<int32_t> output = factory.createArray<int32_t>(dims);

        if (matrix.HasComplexBasis()) {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: matrix.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                output[index][2] = static_cast<int32_t>(re_im_pair.second) + 1;
                ++index;
            }
        } else {
            size_t index = 0;
            for (auto [elem_id, re_im_pair]: matrix.BasisKey()) {
                output[index][0] = static_cast<int32_t>(elem_id);
                output[index][1] = static_cast<int32_t>(re_im_pair.first) + 1; // + 1 for matlab indexing
                ++index;
            }
        }
        return output;
    }

    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<uint64_t>>
    BasisKeyExporter::basis_lists(const Matrix& matrix) {
        // Prepare output lists
        matlab::data::ArrayFactory factory;
        auto output = std::make_pair(
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, matrix.RealBasisIndices().size()}),
                factory.createArray<uint64_t>(matlab::data::ArrayDimensions{1, matrix.ImaginaryBasisIndices().size()})
        );

        auto re_write_iter = output.first.begin();
        for (auto re_basis_elem : matrix.RealBasisIndices()) {
            *re_write_iter = static_cast<int32_t>(re_basis_elem) + 1;
            ++re_write_iter;
        }

        auto im_write_iter = output.second.begin();
        for (auto im_basis_elem : matrix.ImaginaryBasisIndices()) {
            *im_write_iter = static_cast<int32_t>(im_basis_elem) + 1;
            ++im_write_iter;
        }

        return output;
    }

    std::pair<matlab::data::TypedArray<bool>, matlab::data::TypedArray<bool>>
    BasisKeyExporter::basis_masks(const Matrix &matrix) {
        // Prepare masks
        matlab::data::ArrayFactory factory;

        // Create arrays, make them all false
        const auto real_symbol_count = matrix.symbols.Basis.RealSymbolCount();
        const auto imaginary_symbol_count = matrix.symbols.Basis.ImaginarySymbolCount();

        auto output = std::make_pair(
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, real_symbol_count}),
                factory.createArray<bool>(matlab::data::ArrayDimensions{1, imaginary_symbol_count})
        );

        //auto real_mask = matrix.

        std::fill(output.first.begin(), output.first.end(), false);
        std::fill(output.second.begin(), output.second.end(), false);


        for (auto [symbol_id, basis_elems] : matrix.BasisKey()) {
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

}