/**
 * make_representation.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"

namespace Moment::mex::functions {

    class MakeRepresentationParams : public SortedInputs {
    public:
        MakeRepresentationParams(SortedInputs&& raw_inputs);

        uint64_t matrix_system_key = 0;
        uint64_t symmetry_index = 0;
        size_t word_length;

    };

    class MakeRepresentation
            : public ParameterizedMTKFunction<MakeRepresentationParams, MTKEntryPointID::MakeRepresentation> {
    public:
        explicit MakeRepresentation(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void extra_input_checks(MakeRepresentationParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, MakeRepresentationParams &input) override;

    };
}