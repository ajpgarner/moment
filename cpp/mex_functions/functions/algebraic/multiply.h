/**
 * multiply.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */


#include "../../mtk_function.h"

#include "integer_types.h"

#include "import/algebraic_operand.h"

#include <span>
#include <string>
#include <variant>

namespace Moment {
    class Context;
    class MatrixSystem;
}

namespace Moment::mex::functions  {

    struct MultiplyParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        AlgebraicOperand lhs;
        AlgebraicOperand rhs;

    public:
        explicit MultiplyParams(SortedInputs&& inputs);

        enum class OutputMode {
            Unknown,
            MatrixIndex,
            String,
            SymbolCell,
            SequencesWithSymbolInfo
        } output_mode = OutputMode::MatrixIndex;
    };

    class Multiply : public ParameterizedMTKFunction<MultiplyParams, MTKEntryPointID::Multiply> {
    public:
        explicit Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MultiplyParams &input) override;

        void extra_input_checks(MultiplyParams &input) const override;

    };
}
