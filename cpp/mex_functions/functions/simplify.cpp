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
            } else if ((input.input_type == SimplifyParams::InputType::Numbers)
                        || (input.input_type == SimplifyParams::InputType::NumbersArray)) {

                if (input.scalar_input() && (input.operator_string.size() != 1)) {
                    throw_error(engine, errors::internal_error, "Missing operator string.");
                }

                size_t elem_idx = 1; // MATLAB 1-indexing
                for (const auto& op_str : input.operator_string) {
                    size_t idx = 1; // MATLAB 1-indexing
                    for (const auto op_num: op_str) {
                        if ((op_num < 0) || (op_num >= context.size())) {
                            std::stringstream errSS;
                            errSS << "Operator " << (op_num + 1) << " at position " << idx;
                            if (!input.scalar_input()) {
                                errSS << " in index " << elem_idx;
                            }
                            errSS << " is out of range.";
                            throw_error(engine, errors::bad_param, errSS.str());
                        }
                        ++idx;
                    }
                    ++elem_idx;
                }

            } else {
                throw_error(engine, errors::internal_error, "Unknown input type.");
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
            case matlab::data::ArrayType::CELL:
                this->input_type = SimplifyParams::InputType::NumbersArray;
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
            this->operator_string.emplace_back(read_integer_array<oper_name_t>(matlabEngine, "Operator string", inputs[1]));
            for (auto &op: this->operator_string.back()) {
                if (op < 1) {
                    throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
                }
                op -= 1;
            }
            this->input_shape = {1, 1};
        } else if (this->input_type == SimplifyParams::InputType::NumbersArray) {
            const auto input_dims = inputs[1].getDimensions();
            this->input_shape.reserve(input_dims.size());
            std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

            const matlab::data::CellArray as_cell = inputs[1];
            this->operator_string.reserve(as_cell.getNumberOfElements());
            for (auto str : as_cell) {
                this->operator_string.emplace_back(
                        read_integer_array<oper_name_t>(matlabEngine, "Operator string", str));
                for (auto &op: this->operator_string.back()) {
                    if (op < 1) {
                        throw_error(matlabEngine, errors::bad_param, "Operator must be a positive integer.");
                    }
                    op -= 1;
                }
            }

        } else if (this->input_type == SimplifyParams::InputType::String) {
            this->input_shape = {1, 1};
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
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;
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

        if (input.scalar_input()) {
            assert(input.operator_string.size() == 1);
            sequence_storage_t rawOpStr{input.operator_string[0].begin(), input.operator_string[0].end()};
            OperatorSequence opSeq{std::move(rawOpStr), context};

            if (this->verbose) {
                std::stringstream ss;
                sequence_storage_t copyOpStr{input.operator_string[0].begin(), input.operator_string[0].end()};
                ss << context.format_raw_sequence(copyOpStr) << " -> " << opSeq << "\n";
                print_to_console(matlabEngine, ss.str());
            }

            matlab::data::ArrayFactory factory;
            // Export sequence
            output[0] = export_operator_sequence(factory, opSeq);

            // Export minus sign if necessary
            if (output.size() >= 2) {
                output[1] = factory.createScalar<bool>(opSeq.negated());
            }

            // Export hash
            if (output.size() >= 3) {
                output[2] = factory.createScalar<uint64_t>(opSeq.hash());
            }
        } else {

            matlab::data::ArrayFactory factory;

            // Prepare outputs
            auto out_op_seqs = factory.createCellArray(input.input_shape);
            auto out_negation = factory.createArray<bool>(input.input_shape);
            auto out_hashes = factory.createArray<uint64_t>(input.input_shape);

            // Parse and conjugate
            size_t write_index = 0;
            auto out_op_seqs_iter = out_op_seqs.begin();
            auto out_negation_iter = out_negation.begin();
            auto out_hashes_iter = out_hashes.begin();

            std::stringstream ss;
            for (const auto& input_seq : input.operator_string) {
                sequence_storage_t rawOpStr{input_seq.begin(), input_seq.end()};
                OperatorSequence opSeq{std::move(rawOpStr), context};

                if (this->verbose) {
                    sequence_storage_t copyOpStr{input.operator_string[0].begin(), input.operator_string[0].end()};
                    ss << context.format_raw_sequence(copyOpStr) << " -> " << opSeq << "\n";
                }

                *out_op_seqs_iter = export_operator_sequence(factory, opSeq);
                *out_negation_iter = opSeq.negated();
                *out_hashes_iter = opSeq.hash();

                // Next:
                ++out_op_seqs_iter;
                ++out_negation_iter;
                ++out_hashes_iter;
                ++write_index;
            }

            if (this->verbose) {
                print_to_console(matlabEngine, ss.str());
            }

            // Move outputs
            output[0] = std::move(out_op_seqs);
            if (output.size() >= 2) {
                output[1] = std::move(out_negation);
            }
            if (output.size() >= 3) {
                output[2] = std::move(out_hashes);
            }

        }
    }

}
