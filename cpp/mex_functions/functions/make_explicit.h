/**
 * make_explicit.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once
#include "../mex_function.h"

#include <vector>

namespace Moment {
    namespace Inflation {
        class InflationMatrixSystem;
    }
    namespace Locality {
        class LocalityMatrixSystem;
    }
}

namespace Moment::mex::functions {

    struct MakeExplicitParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The reference to the matrix within the system. */
        uint64_t matrix_index = 0;

        /** The type of input requested */
        enum class InputType {
            AllMeasurements,
            SpecifiedMeasurement
        } input_type = InputType::AllMeasurements;

        /** The requested measurements / observables */
        std::vector<std::pair<uint64_t, uint64_t>> measurements_or_observables;

        /** The supplied values */
        std::vector<double> values;


    public:
        explicit MakeExplicitParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);

    };

    class MakeExplicit : public ParameterizedMexFunction<MakeExplicitParams, MEXEntryPointID::MakeExplicit> {
    public:
        explicit MakeExplicit(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MakeExplicitParams &input) override;

        void extra_input_checks(MakeExplicitParams &input) const override;

    private:
        matlab::data::Array do_make_explicit(const Inflation::InflationMatrixSystem& ims, MakeExplicitParams &input);

        matlab::data::Array do_make_explicit(const Locality::LocalityMatrixSystem& lms, MakeExplicitParams &input);

    };

}