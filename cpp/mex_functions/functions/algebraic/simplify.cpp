/**
 * simplify.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "simplify.h"

#include "errors.h"
#include "storage_manager.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/name_table.h"

#include "export/export_operator_sequence.h"
#include "export/export_polynomial.h"

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
                    throw BadParameter{"String-based operator input is only supported for algebraic scenarios."};
                }
                assert(acPtr);
                const auto& names = acPtr->names();
                input.operator_string.emplace_back();
                auto& op_seq = input.operator_string.back();
                op_seq.reserve(input.named_operators.size());
                size_t idx = 1; // MATLAB 1-indexing
                for (const auto& opStr : input.named_operators) {
                    try {
                        op_seq.emplace_back(names.find(opStr));
                    } catch (const std::invalid_argument& iae) {
                        std::stringstream errSS;
                        errSS << "Could not parse operator \"" << opStr << "\" at index " << idx << ".";
                        throw BadParameter{errSS.str()};
                    }
                    ++idx;
                }
            } else if ((input.input_type == SimplifyParams::InputType::Numbers)
                        || (input.input_type == SimplifyParams::InputType::NumbersArray)) {

                if (input.scalar_input() && (input.operator_string.size() != 1)) {
                    throw InternalError{"Missing operator string."};
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
                            throw BadParameter{errSS.str()};
                        }
                        ++idx;
                    }
                    ++elem_idx;
                }

            } else {
                throw InternalError{"Unknown input type."};
            }
        }
    }

    SimplifyParams::SimplifyParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)}, matrix_system_key{matlabEngine} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        const bool polynomial_mode = this->flags.contains(u"polynomial");
        if (polynomial_mode) {
            this->parse_as_polynomial();
        } else {
            this->parse_as_operators();
        }

        if (this->flags.contains(u"string_out")) {
            this->output_mode = OutputMode::String;
        }
    }

    void SimplifyParams::parse_as_polynomial() {
        this->input_type = SimplifyParams::InputType::SymbolCell;
        if (inputs[1].getType() != matlab::data::ArrayType::CELL) {
            throw BadParameter{"Polynomial mode expects symbol cell input."};
        }

        const auto input_dims = inputs[1].getDimensions();
        this->input_shape.reserve(input_dims.size());
        std::copy(input_dims.cbegin(), input_dims.cend(), std::back_inserter(this->input_shape));

        this->rawPolynomials.reserve(inputs[1].getNumberOfElements());

        // Looks suspicious, but promised by MATLAB to be a reference, not copy.
        const matlab::data::CellArray cell_input = inputs[1];
        auto read_iter = cell_input.begin();
        while (read_iter != cell_input.end()) {
            this->rawPolynomials.emplace_back(read_raw_polynomial_data(this->matlabEngine, "Input", *read_iter));
            ++read_iter;
        }
    }

    void SimplifyParams::parse_as_operators() {

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
                    throw BadParameter{"Operator must be a positive integer."};
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
                        throw BadParameter{"Operator must be a positive integer."};
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
            throw BadParameter{"Operator sequence must be an array of numbers or of (string) names."};
        }
    }

    Simplify::Simplify(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"string_out");
        this->flag_names.emplace(u"polynomial");
    }

    void Simplify::operator()(IOArgumentRange output, SimplifyParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.matrix_system_key(this->storageManager);
        assert(matrixSystemPtr); // ^-- should throw if not found

        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto lock = matrixSystem.get_read_lock();

        if (input.input_type == SimplifyParams::InputType::SymbolCell) {
            this->simplify_polynomials(output, input, matrixSystem);
        } else {
            if (input.scalar_input()) {
                this->simplify_operator(output, input, matrixSystem);
            } else {
                this->simplify_operator_array(output, input, matrixSystem);
            }
        }

    }

    void Simplify::simplify_operator(IOArgumentRange& output, SimplifyParams &input,
                                      const MatrixSystem &matrixSystem) {

        const Context& context = matrixSystem.Context();
        process_input_string(matlabEngine, context, input);
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
            output[1] = factory.createScalar<std::complex<double>>(to_scalar(opSeq.get_sign()));
        }

        // Export hash
        if (output.size() >= 3) {
            output[2] = factory.createScalar<uint64_t>(opSeq.hash());
        }
    }

    void Simplify::simplify_operator_array(IOArgumentRange& output, SimplifyParams &input,
                                      const MatrixSystem &matrixSystem) {
        const Context& context = matrixSystem.Context();
        process_input_string(matlabEngine, context, input);

        // Prepare outputs
        matlab::data::ArrayFactory factory;
        auto out_op_seqs = factory.createCellArray(input.input_shape);
        auto out_signs = factory.createArray<std::complex<double>>(input.input_shape);
        auto out_hashes = factory.createArray<uint64_t>(input.input_shape);

        // Parse and conjugate
        size_t write_index = 0;
        auto out_op_seqs_iter = out_op_seqs.begin();
        auto out_sign_iter = out_signs.begin();
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
            *out_sign_iter = to_scalar(opSeq.get_sign());
            *out_hashes_iter = opSeq.hash();

            // Next:
            ++out_op_seqs_iter;
            ++out_sign_iter;
            ++out_hashes_iter;
            ++write_index;
        }

        if (this->verbose) {
            print_to_console(matlabEngine, ss.str());
        }

        // Move outputs
        output[0] = std::move(out_op_seqs);
        if (output.size() >= 2) {
            output[1] = std::move(out_signs);
        }
        if (output.size() >= 3) {
            output[2] = std::move(out_hashes);
        }
    }

    void Simplify::simplify_polynomials(IOArgumentRange& output, SimplifyParams &input,
                                        const MatrixSystem &matrixSystem) {
        // Check outputs
        if (output.size() != 1) {
            throw OutputCountException{"simplify", 1, 1, output.size(),
                                       "Polynomial simplification expects single output."};
        }

        const auto& poly_factory = matrixSystem.polynomial_factory();

        // Read (and simplify) inputs
        std::vector<Polynomial> polynomials;
        polynomials.reserve(input.rawPolynomials.size());
        for (const auto& input_poly : input.rawPolynomials) {
            polynomials.emplace_back(raw_data_to_polynomial(this->matlabEngine, poly_factory, input_poly));
        }


        // Export
        matlab::data::ArrayFactory factory;
        PolynomialExporter exporter{this->matlabEngine, factory,
                                    matrixSystem.Context(), matrixSystem.Symbols(), poly_factory.zero_tolerance};
        if (input.output_mode == SimplifyParams::OutputMode::String) {
            matlab::data::StringArray string_out = factory.createArray<matlab::data::MATLABString>(input.input_shape);

            std::transform(polynomials.cbegin(), polynomials.cend(), string_out.begin(),
                           [&exporter](const Polynomial &poly) -> matlab::data::MATLABString {
                               return exporter.string(poly);
                           });
            output[0] = std::move(string_out);
        } else {
            matlab::data::CellArray cell_out = factory.createCellArray(input.input_shape);
            std::transform(polynomials.cbegin(), polynomials.cend(), cell_out.begin(),
                           [&exporter](const Polynomial &poly) -> matlab::data::CellArray {
                               return exporter.symbol_cell(poly);
                           });
            output[0] = std::move(cell_out);
        }
    }

}
