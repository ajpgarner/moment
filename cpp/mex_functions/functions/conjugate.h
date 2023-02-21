/**
 * transpose.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../mex_function.h"

#include "integer_types.h"

#include <vector>

namespace Moment::mex::functions {

    struct ConjugateParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The operator string to conjugate. */
        std::vector<oper_name_t> operator_string;

    public:
        explicit ConjugateParams(SortedInputs&& structuredInputs);

    };

    class Conjugate : public ParameterizedMexFunction<ConjugateParams, MEXEntryPointID::Conjugate> {
    public:
        explicit Conjugate(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ConjugateParams &input) override;

        void extra_input_checks(ConjugateParams &input) const override;
    };

}