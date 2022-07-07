/**
 * probability_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"


namespace NPATK::mex::functions {

    struct ProbabilityTableParams : public SortedInputs {
    public:
        /** The reference to the moment matrix, if one is requested */
        uint64_t moment_matrix_key = 0;

    public:
        explicit ProbabilityTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);
    };

    class ProbabilityTable : public MexFunction {
    public:
        explicit ProbabilityTable(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };

}