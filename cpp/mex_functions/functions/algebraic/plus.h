/**
 * plus.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../../mtk_function.h"
#include "integer_types.h"

#include "import/algebraic_operand.h"
#include "import/matrix_system_id.h"

#include <span>
#include <string>

namespace Moment::mex::functions  {

    struct PlusParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** Left hand operand */
        AlgebraicOperand lhs;

        /** Right hand operand */
        AlgebraicOperand rhs;

        enum class OutputMode {
            Unknown,
            MatrixID,
            String,
            SymbolCell,
            SequencesWithSymbolInfo
        } output_mode = OutputMode::SymbolCell;

    public:
        explicit PlusParams(SortedInputs&& inputs);
    };

    class Plus : public ParameterizedMTKFunction<PlusParams, MTKEntryPointID::Plus> {
    public:
        explicit Plus(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, PlusParams &input) override;
    };
}
