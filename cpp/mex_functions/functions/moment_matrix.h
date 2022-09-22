/**
 * moment_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "matlab_classes/scenario.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions  {

    struct MomentMatrixParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        unsigned long hierarchy_level = 0;

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
        explicit MomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);
    };

    class MomentMatrix : public NPATK::mex::functions::MexFunction {
    public:
        explicit MomentMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}
