/**
 * make_moment_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions  {

    struct MakeMomentMatrixParams : public SortedInputs {
    public:
        /** True if operator sequence strings should be output, false for symbols */
        bool output_sequences = false;

        unsigned long hierarchy_level = 0;

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

        matlab::data::Array* ptrSettings = nullptr;

    public:
        explicit MakeMomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

        friend class MakeMomentMatrix;

        [[nodiscard]] std::string to_string() const override;

    private:
        void getFlatFromParams(matlab::engine::MATLABEngine &matlabEngine);
        void getFlatFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void verifyAsContext(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array& input);
    };

    class MakeMomentMatrix : public NPATK::mex::functions::MexFunction {
    public:
        explicit MakeMomentMatrix(matlab::engine::MATLABEngine& matlabEngine);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}
