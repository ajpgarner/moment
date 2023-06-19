/**
 * export_matrix_basis_masks.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "exporter.h"

#include "MatlabDataArray.hpp"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class SymbolTable;
    class Matrix;
}

namespace Moment::mex {

    class BasisKeyExporter : public Exporter {
    public:

        BasisKeyExporter(matlab::engine::MATLABEngine &engine)
            : Exporter{engine} { }

        /**
         * Outputs as list of symbols associated with a matrix, and their corresponding real and imaginary indices.
         * @param matrix The symbolic matrix.
         * @return 2xn or 3xn matlab array. First column are symbol ids, second real indices, third imaginary indices.
         */
        matlab::data::TypedArray<int32_t> basis_key(const Matrix &matrix);

        /**
         * Outputs a list of real and imaginary symbols associated with a matrix.
         * @param matrix The symbolic matrix.
         * @return Pair of matlab arrays, first for real symbols, second for imaginary symbols.
         */
        std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<uint64_t>>
        basis_lists(const Matrix& matrix);

        /**
         * Outputs masks for the real and imaginary symbols associated with a matrix.
         * If symbol i features in the matrix, element i-1 will have the value true. (In ML indexing, this corresponds to
         * element i); otherwise the value will be false. Symbol "0" is always omitted, as it never contributes to a basis.
         * @param matrix The symbolic matrix.
         * @return Pair of matlab arrays, first for real symbols, second for imaginary symbols.
         */
        std::pair<matlab::data::TypedArray<bool>, matlab::data::TypedArray<bool>>
        basis_masks(const Matrix& matrix);
    };

}