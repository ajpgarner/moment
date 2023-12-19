/**
 * echo_operand.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "echo_operand.h"
#include "storage_manager.h"
#include "utilities/read_as_scalar.h"

namespace Moment::mex::functions  {

    EchoOperandParams::EchoOperandParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)},
            matrix_system_key{matlabEngine},  operand{matlabEngine, "Operand"} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        const bool expect_symbols = this->flags.contains(u"symbolic");
        this->operand.parse_input(this->inputs[1], expect_symbols);
    }

    EchoOperand::EchoOperand(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 0;
        this->max_outputs = 1;
        this->flag_names.emplace(u"symbolic");
    }

    void EchoOperand::operator()(IOArgumentRange output, EchoOperandParams &input) {
        throw_error(matlabEngine, errors::internal_error, "Echo operand not yet implemented.");

    }
}