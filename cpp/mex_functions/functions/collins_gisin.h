/**
 * collins_gisin.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"


namespace Moment::mex::functions  {

    struct CollinsGisinParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        enum class OutputType {
            RealBasis,
            SymbolIds,
            Sequences
        } outputType = OutputType::RealBasis;

    public:
        explicit CollinsGisinParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

    };

    class CollinsGisin : public Moment::mex::functions::MexFunction {
    public:
        explicit CollinsGisin(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}
