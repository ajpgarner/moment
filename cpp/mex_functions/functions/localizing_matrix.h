/**
 * localizing_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "integer_types.h"
#include "operators/matrix/localizing_matrix_index.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions  {

    struct LocalizingMatrixParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        unsigned long hierarchy_level = 0;

        std::vector<oper_name_t> localizing_word;

        enum class OutputMode {
            /** Unknown output */
            Unknown = 0,
            /** Output dimension of matrix */
            DimensionOnly,
            /** Output matrix of symbol names */
            Symbols,
            /** Output matrix of string representation of operator sequences */
            Sequences
        } output_mode = OutputMode::Unknown;

    public:
        explicit LocalizingMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

        /**
         * Use the supplied context to create an index for the requested localizing matrix.
         */
        LocalizingMatrixIndex to_index(matlab::engine::MATLABEngine &matlabEngine, const Context& context) const;
    };

    class LocalizingMatrix : public NPATK::mex::functions::MexFunction {
    public:
        explicit LocalizingMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}
