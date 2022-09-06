/**
 * make_moment_matrix.h
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

    struct MakeMomentMatrixParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        unsigned long hierarchy_level = 0;

        enum class OutputMode {
            Unknown = 0,
            Symbols,
            Sequences,
            TableOnly
        } output_mode = OutputMode::Unknown;

    public:
        explicit MakeMomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

        friend class MakeMomentMatrix;
    };

    class MakeMomentMatrix : public NPATK::mex::functions::MexFunction {
    public:
        explicit MakeMomentMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}
