/**
 * locality_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions {
    struct LocalityMatrixSystemParams : public SortedInputs {
    public:
        size_t total_operators = 0;
        size_t number_of_parties = 1;
        size_t total_measurements = 0;
        std::vector<size_t> mmts_per_party;
        std::vector<size_t> outcomes_per_mmt;

        /** The 'precision' limit, such that if a value is less than eps * [this value] it is treated as zero. */
        double zero_tolerance = 1.0;

    public:
        explicit LocalityMatrixSystemParams(SortedInputs &&inputs);

    private:
        void getFromParams();

        void getFromInputs();

        void readMeasurementSpecification( matlab::data::Array& input, const std::string& paramName);

        void readOutcomeSpecification(matlab::data::Array& input, const std::string& paramName);

    };


    class LocalityMatrixSystem : public ParameterizedMexFunction<LocalityMatrixSystemParams,
                                                                 MEXEntryPointID::LocalityMatrixSystem> {
    public:
        explicit LocalityMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, LocalityMatrixSystemParams &input) override;


    };
}