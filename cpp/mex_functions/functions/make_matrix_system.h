/**
 * make_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "matlab_classes/scenario.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions {

    struct MakeMatrixSystemParams : public SortedInputs {
    public:
        unsigned long number_of_parties = 0;

        enum class SpecificationMode {
            Unknown = 0,
            FlatNoMeasurements,
            FlatWithMeasurements,
            FromSettingObject
        } specification_mode = SpecificationMode::Unknown;

        unsigned long flat_mmts_per_party = 0;
        unsigned long flat_outcomes_per_mmt = 0;
        unsigned long flat_operators_per_party = 0;

        std::unique_ptr<classes::Scenario> settingPtr;

    public:
        explicit MakeMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs);

        friend class MakeMatrixSystem;

        [[nodiscard]] std::string to_string() const override;

    private:
        void getFlatFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFlatFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void getSettingObject(matlab::engine::MATLABEngine &matlabEngine, matlab::data::Array &input);
    };


    class MakeMatrixSystem : public NPATK::mex::functions::MexFunction {
    public:
        explicit MakeMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}