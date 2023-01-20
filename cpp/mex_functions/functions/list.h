/**
 * list.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace Moment::mex::functions  {

    struct ListParams : public SortedInputs {
    public:
        enum class OutputType {
            All,
            OneSystem
        } output_type = OutputType::All;

        uint64_t matrix_system_key = 0;

    public:
        explicit ListParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

    };

    class List : public Moment::mex::functions::MexFunction {
    public:
        explicit List(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}
