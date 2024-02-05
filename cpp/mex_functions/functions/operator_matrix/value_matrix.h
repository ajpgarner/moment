/**
 * value_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "operator_matrix.h"
#include "import/algebraic_operand.h"

#include <optional>
#include <string>

namespace Moment {
    class Context;
}

namespace Moment::mex::functions  {

    struct ValueMatrixParams : public OperatorMatrixParams {
    public:
        AlgebraicOperand raw_data;

        std::optional<std::string> label;

    public:
        explicit ValueMatrixParams(SortedInputs&& inputs); // : OperatorMatrixParams(std::move(inputs)) { }

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        void parse_optional_params();

        void load_numeric_array(const matlab::data::Array& input);

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 2; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, numeric data]"; }
    };

    class ValueMatrix
            : public Moment::mex::functions::OperatorMatrix<ValueMatrixParams, MTKEntryPointID::ValueMatrix> {
    public:
        ValueMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}
