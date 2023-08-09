/**
 * apply_moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "apply_moment_rules.h"

#include "matrix_system/matrix_system.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/polynomial_factory.h"

#include "export/export_polynomial.h"

#include "utilities/read_choice.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include "storage_manager.h"

#include <sstream>

namespace Moment::mex::functions {

    ApplyMomentRulesParams::ApplyMomentRulesParams(SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Get rulebook index
        this->rulebook_index = read_positive_integer<uint64_t>(matlabEngine, "Rulebook index", this->inputs[1], 0);

        // Read symbol combo cell
        read_symbol_cell_input(this->inputs[2]);

        // Read input mode, if set
        auto inputModeIter = this->params.find(u"input");
        if (inputModeIter  != this->params.end()) {
            switch (read_choice("input", {"symbols", "sequences"}, inputModeIter->second)) {
                case 0:
                    this->input_format = InputFormat::SymbolCell;
                    break;
                case 1:
                    this->input_format = InputFormat::OperatorCell;
                    throw_error(this->matlabEngine, errors::bad_param, "Operator input mode not yet supported.");
                    break;
                default:
                    throw_error(this->matlabEngine, errors::bad_param, "Unknown input mode.");
            }
        }

        // Read output mode, if set
        auto outputModeIter = this->params.find(u"output");
        if (outputModeIter != this->params.end()) {
            switch (read_choice("output", {"string", "symbols", "polynomials"}, outputModeIter->second)) {
                case 0:
                    this->output_format = OutputFormat::String;
                    break;
                case 1:
                    this->output_format = OutputFormat::SymbolCell;
                    break;
                case 2:
                    this->output_format = OutputFormat::Polynomial;
                    break;
                default:
                    throw_error(this->matlabEngine, errors::bad_param, "Unknown output mode.");
            }
        }
    }

    void ApplyMomentRulesParams::read_symbol_cell_input(const matlab::data::Array &array) {
        if (array.getType() != matlab::data::ArrayType::CELL) {
            throw_error(this->matlabEngine, errors::bad_param, "Expected cell array input.");
        }

        // Read dimensions
        const matlab::data::CellArray as_cell = array;
        auto dims = as_cell.getDimensions();
        this->input_shape.reserve(dims.size());
        std::copy(dims.begin(), dims.end(), std::back_inserter(this->input_shape));

        // Read polynomials
        size_t offset = 1;
        this->raw_polynomial.reserve(as_cell.getNumberOfElements());
        for (const auto& elem : as_cell) {
            std::stringstream ss;
            ss << "Polynomial at index " << offset;
            this->raw_polynomial.emplace_back(read_raw_polynomial_data(this->matlabEngine, ss.str(), elem));
            ++offset;
        }
    }

    void ApplyMomentRules::extra_input_checks(ApplyMomentRulesParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    ApplyMomentRules::ApplyMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.insert(u"input");
        this->param_names.insert(u"output");
    }

    void ApplyMomentRules::operator()(IOArgumentRange output, ApplyMomentRulesParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        assert(matrixSystemPtr); // ^-- should throw if not found

        // Read-lock matrix system
        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto lock = matrixSystem.get_read_lock();
        const auto& context = matrixSystem.Context();
        const auto& symbols = matrixSystem.Symbols();

        // Retrieve rules, or throw
        const auto& rulebook = [&]() -> const MomentRulebook& {
            try {
                return matrixSystem.Rulebook(input.rulebook_index); // <- throws, if not found.
            } catch (const Moment::errors::missing_component& mce) {
                std::stringstream errSS;
                errSS << "Could not find rulebook at index " << input.rulebook_index << ".";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
        }();
        const auto& factory = rulebook.factory;

        // Convert input to polynomials
        std::vector<Polynomial> input_polynomials;
        input_polynomials.reserve(input.raw_polynomial.size());
        for (const auto& raw_poly : input.raw_polynomial) {
            input_polynomials.emplace_back(raw_data_to_polynomial(this->matlabEngine, factory, raw_poly));
        }

        // Transform input polynomials
        std::vector<Polynomial> output_polynomials;
        output_polynomials.reserve(input_polynomials.size());
        for (const auto& input_poly : input_polynomials) {
            output_polynomials.emplace_back(rulebook.reduce(input_poly));
        }

        matlab::data::ArrayFactory mlfactory;
        PolynomialExporter polynomialExporter{this->matlabEngine, mlfactory,
                                              context, symbols, factory.zero_tolerance};

        switch (input.output_format) {
            case ApplyMomentRulesParams::OutputFormat::SymbolCell: {
                matlab::data::CellArray cell_out = mlfactory.createCellArray(input.input_shape);
                std::transform(output_polynomials.cbegin(), output_polynomials.cend(), cell_out.begin(),
                               [&polynomialExporter](const Polynomial &poly) -> matlab::data::CellArray {
                                   return polynomialExporter.symbol_cell(poly);
                               });
                output[0] = std::move(cell_out);
            }
                break;
            case ApplyMomentRulesParams::OutputFormat::Polynomial: {
                matlab::data::CellArray cell_out = mlfactory.createCellArray(input.input_shape);
                std::transform(output_polynomials.cbegin(), output_polynomials.cend(), cell_out.begin(),
                               [&mlfactory, &polynomialExporter](const Polynomial &poly) -> matlab::data::CellArray {
                                   auto fpi = polynomialExporter.sequences(poly, true);
                                   return fpi.move_to_cell(mlfactory);
                               });
                output[0] = std::move(cell_out);
            }
                break;
            case ApplyMomentRulesParams::OutputFormat::String: {
                matlab::data::TypedArray<matlab::data::MATLABString> string_out
                    = mlfactory.createArray<matlab::data::MATLABString>(input.input_shape);
                std::transform(output_polynomials.cbegin(), output_polynomials.cend(), string_out.begin(),
                               [&polynomialExporter](const Polynomial &poly) -> matlab::data::MATLABString {
                                   return polynomialExporter.string(poly, true);
                               });
                output[0] = std::move(string_out);
            }
                break;
            default:
                throw_error(this->matlabEngine, errors::bad_param, "Unknown output format.");
        }

    }

}