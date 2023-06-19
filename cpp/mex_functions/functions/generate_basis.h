/**
 * generate_basis.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../mex_function.h"

#include "matrix/operator_matrix/operator_matrix.h"


namespace Moment::mex::functions {

    struct GenerateBasisParams : public SortedInputs {
    public:
        /** True, if output should be a sparse matrix */
        bool sparse_output = false;

        /** True, if output should be an indexed sparse array, or a flattened monolithic array */
        bool monolithic_output = false;

        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The reference to the matrix within the system. */
        uint64_t matrix_index = 0;

    public:
        explicit GenerateBasisParams(SortedInputs&& structuredInputs);

    };

class GenerateBasis : public ParameterizedMexFunction<GenerateBasisParams, MEXEntryPointID::GenerateBasis> {
    public:
        explicit GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

protected:
    void operator()(IOArgumentRange output, GenerateBasisParams &input) override;

    void extra_input_checks(GenerateBasisParams &input) const override;
};

}