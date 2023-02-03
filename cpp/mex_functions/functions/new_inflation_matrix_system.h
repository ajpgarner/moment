/**
 * new_inflation_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../mex_function.h"

#include "integer_types.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions {
    struct NewInflationMatrixSystemParams : public SortedInputs {
    public:
        std::vector<size_t> outcomes_per_observable{};
        std::vector<std::set<oper_name_t>> source_init_list{};
        size_t inflation_level = 1;

    public:
        explicit NewInflationMatrixSystemParams(SortedInputs &&inputs);

        [[nodiscard]] std::string to_string() const override;

    private:
        void getFromParams();

        void getFromInputs();

        void readSourceCell(size_t num_observables, const matlab::data::Array& input);

    };

    class NewInflationMatrixSystem
        : public ParameterizedMexFunction<NewInflationMatrixSystemParams, MEXEntryPointID::NewInflationMatrixSystem> {
    public:
        explicit NewInflationMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, NewInflationMatrixSystemParams &input) override;

    };
}