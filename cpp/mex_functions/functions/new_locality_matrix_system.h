/**
 * new_locality_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mex_function.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions {
    struct NewLocalityMatrixSystemParams : public SortedInputs {
    public:
        size_t total_operators = 0;
        size_t number_of_parties = 1;
        size_t total_measurements = 0;
        std::vector<size_t> mmts_per_party;
        std::vector<size_t> outcomes_per_mmt;

    public:
        explicit NewLocalityMatrixSystemParams(SortedInputs &&inputs);

    private:
        void getFromParams();

        void getFromInputs();

        void readMeasurementSpecification( matlab::data::Array& input, const std::string& paramName);

        void readOutcomeSpecification(matlab::data::Array& input, const std::string& paramName);

    };


    class NewLocalityMatrixSystem : public ParameterizedMexFunction<NewLocalityMatrixSystemParams, MEXEntryPointID::NewLocalityMatrixSystem> {
    public:
        explicit NewLocalityMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, NewLocalityMatrixSystemParams &input) override;


    };
}