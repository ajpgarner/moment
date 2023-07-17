/**
 * inflation_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"

#include "integer_types.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions {
    struct InflationMatrixSystemParams : public SortedInputs {
    public:
        std::vector<size_t> outcomes_per_observable{};
        std::vector<std::set<oper_name_t>> source_init_list{};
        size_t inflation_level = 1;
        double zero_tolerance = 1.0;

    public:
        explicit InflationMatrixSystemParams(SortedInputs &&inputs);

        [[nodiscard]] std::string to_string() const override;

    private:
        void getFromParams();

        void getFromInputs();

        void readSourceCell(size_t num_observables, const matlab::data::Array& input);

    };

    class InflationMatrixSystem
        : public ParameterizedMexFunction<InflationMatrixSystemParams, MEXEntryPointID::InflationMatrixSystem> {
    public:
        explicit InflationMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, InflationMatrixSystemParams &input) override;

    };
}