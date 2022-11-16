/**
 * new_inflation_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

#include "integer_types.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions {
    struct NewInflationMatrixSystemParams : public SortedInputs {
    public:
        std::vector<size_t> outcomes_per_observable{};
        std::vector<std::set<oper_name_t>> source_init_list{};
        size_t inflation_level = 1;

    public:
        explicit NewInflationMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs);

        [[nodiscard]] std::string to_string() const override;

    private:
        void getFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void readSourceCell(matlab::engine::MATLABEngine &matlabEngine, size_t num_observables,
                            const matlab::data::Array& input);

    };


    class NewInflationMatrixSystem : public NPATK::mex::functions::MexFunction {
    public:
        explicit NewInflationMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}