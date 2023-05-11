/**
 * simplify.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "simplify.h"

#include "storage_manager.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/name_table.h"

#include "export/export_operator_sequence.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"


#include <sstream>

namespace Moment::mex::functions {

    namespace {
        void process_input_string(matlab::engine::MATLABEngine& engine, const Context& context, SimplifyParams& input) {
            if (input.input_type == SimplifyParams::InputType::String) {
                const auto * acPtr = dynamic_cast<const Algebraic::AlgebraicContext*>(&context);
                if (nullptr == acPtr) {
                    throw_error(engine, errors::bad_param,
                                "String-based operator input is only supported for algebraic scenarios.");
                }
                assert(acPtr);
                const auto& names = acPtr->names();
                input.operator_string.reserve(input.named_operators.size());
                size_t idx = 1; // MATLAB 1-indexing
                for (const auto& opStr : input.named_operators) {
                    try {
                        input.operator_string.emplace_back(names.find(opStr));
                    } catch (const std::invalid_argument& iae) {
                        std::stringstream errSS;
                        errSS << "Could not parse operator \"" << opStr << "\" at index " << idx << ".";
                        throw_error(engine, errors::bad_param, errSS.str());
                    }
                    ++idx;
                }
            } else {
                for (auto op_num : input.operator_string) {
                    size_t idx = 1; // MATLAB 1-indexing
                    if ((op_num < 0) || (op_num >= context.size())) {
                        std::stringstream errSS;
                        errSS << "Operator " << (op_num+1) << " at index " << idx << " is out of range.";
                        throw_error(engine, errors::bad_param, errSS.str());
                    }
                    ++idx;
                }
            }
        }
    }

    SimplifyParams::SimplifyParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        switch (inputs[1].getType()) {
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                this->input_type = SimplifyParams::InputType::Numbers;
                break;
            case matlab::data::ArrayType::MATLAB_STRING:
            case matlab::data::ArrayType::CHAR:
                this->input_type = SimplifyParams::InputType::String;
                break;
            default:
                this->input_type = SimplifyParams::InputType::Unknown;
                break;
        }

        // Read numerically, if numbers provided
        if (this->input_type == SimplifyParams::InputType::Numbers) {

            // Read op string, translate from MATLAB to C++ indexing
            this->operator_string = read_integer_array<oper_name_t>(matlabEngine, "Operator string", inputs[1]);
            for (auto &op: this->operator_string) {
                if (op < 1) {
                    throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
                }
                op -= 1;
            }
        } else if (this->input_type == SimplifyParams::InputType::String) {
            this->named_operators.reserve(inputs[1].getNumberOfElements());

            // Pre-process string for later parsing
            if (inputs[1].getType() == matlab::data::ArrayType::MATLAB_STRING) {
                matlab::data::TypedArray<matlab::data::MATLABString> mlsArray = inputs[1];
                for (const auto& elem : mlsArray) {
                    if (elem.has_value()) {
                        this->named_operators.emplace_back(elem);
                    } else {
                        this->named_operators.emplace_back();
                    }
                }
            } else {
                auto name_char_array = static_cast<matlab::data::CharArray>(inputs[1]);
                auto name_str = name_char_array.toAscii();
                for (auto cx : name_str) {
                    this->named_operators.emplace_back(1, cx);
                }
            }

        } else {
            throw_error(matlabEngine, errors::bad_param, "Operator sequence must be an array of numbers or of (string) names.");
        }
    }

    void Simplify::extra_input_checks(SimplifyParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Simplify::Simplify(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction(matlabEngine, storage, u"simplify") {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void Simplify::operator()(IOArgumentRange output, SimplifyParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto lock = matrixSystem.get_read_lock();
        const Context& context = matrixSystem.Context();

        process_input_string(matlabEngine, context, input);

        sequence_storage_t rawOpStr{input.operator_string.begin(), input.operator_string.end()};
        OperatorSequence opSeq{std::move(rawOpStr), context};

        if (this->verbose) {
            std::stringstream ss;
            ss << context.format_raw_sequence(rawOpStr) << " -> " << opSeq << "\n";
            print_to_console(matlabEngine, ss.str());
        }

        matlab::data::ArrayFactory factory;
        output[0] = export_operator_sequence(factory, opSeq);
    }

}
