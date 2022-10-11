/**
 * new_locality_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions {

    struct NewLocalityMatrixSystemParams : public SortedInputs {
    public:
        size_t total_operators = 0;
        size_t number_of_parties = 1;
        size_t total_measurements = 0;
        std::vector<size_t> mmts_per_party;
        std::vector<size_t> outcomes_per_mmt;

    public:
        explicit NewLocalityMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs);

    private:
        void getFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void readMeasurementSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                          matlab::data::Array& input, const std::string& paramName);

        void readOutcomeSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                      matlab::data::Array& input, const std::string& paramName);

    };


    class NewLocalityMatrixSystem : public NPATK::mex::functions::MexFunction {
    public:
        explicit NewLocalityMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };



}