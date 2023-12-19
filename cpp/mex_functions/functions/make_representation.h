/**
 * make_representation.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"

#include "import/matrix_system_id.h"

namespace Moment::mex::functions {

    class MakeRepresentationParams : public SortedInputs {
    public:

        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;
        size_t word_length;

        MakeRepresentationParams(SortedInputs&& raw_inputs);

    };

    class MakeRepresentation
            : public ParameterizedMTKFunction<MakeRepresentationParams, MTKEntryPointID::MakeRepresentation> {
    public:
        explicit MakeRepresentation(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MakeRepresentationParams &input) override;

    };
}