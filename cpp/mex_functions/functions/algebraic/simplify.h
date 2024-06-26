/**
 * simplify.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "mtk_function.h"


#include "import/matrix_system_id.h"
#include "import/read_polynomial.h"

#include "integer_types.h"

#include <vector>

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions {

    struct SimplifyParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** The operator string to simplify. */
        std::vector<std::vector<oper_name_t>> operator_string;

        /** Operators, as UTF-8 strings, if provided */
        std::vector<std::string> named_operators;

        enum class InputType {
            Unknown,
            Numbers,
            NumbersArray,
            String
        } input_type = InputType::Unknown;

        enum class OutputMode {
            Default,
            String
        } output_mode = OutputMode::Default;

        std::vector<size_t> input_shape;

    public:
        explicit SimplifyParams(SortedInputs&& structuredInputs);

        [[nodiscard]] bool scalar_input() const noexcept { return this->input_type != InputType::NumbersArray; }

    private:
        void parse_as_operators();
    };

    class Simplify : public ParameterizedMTKFunction<SimplifyParams, MTKEntryPointID::Simplify> {
    public:
        explicit Simplify(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, SimplifyParams &input) override;

    private:
        void simplify_operator(IOArgumentRange& output, SimplifyParams& input, const MatrixSystem& matrixSystem);

        void simplify_operator_array(IOArgumentRange& output, SimplifyParams& input, const MatrixSystem& matrixSystem);
    };

}