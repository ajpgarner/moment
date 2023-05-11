/**
 * simplify.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../mex_function.h"

#include "integer_types.h"

#include <vector>

namespace Moment::mex::functions {

    struct SimplifyParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The operator string to simplify. */
        std::vector<oper_name_t> operator_string;

        /** Operators, as UTF-8 strings, if provided */
        std::vector<std::string> named_operators;

        enum class InputType {
            Unknown,
            Numbers,
            String
        } input_type = InputType::Unknown;

    public:
        explicit SimplifyParams(SortedInputs&& structuredInputs);

    };

    class Simplify : public ParameterizedMexFunction<SimplifyParams, MEXEntryPointID::Simplify> {
    public:
        explicit Simplify(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, SimplifyParams &input) override;

        void extra_input_checks(SimplifyParams &input) const override;
    };

}