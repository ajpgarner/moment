/**
 * apply_moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "apply_moment_rules.h"

#include "matrix_system.h"
#include "symbolic/moment_substitution_rulebook.h"

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
        this->raw_polynomial = read_raw_polynomial_data(this->matlabEngine, "Polynomial", this->inputs[2]);

        // Read output mode, if set
        auto outputModeIter = this->params.find(u"output");
        if (outputModeIter != this->params.end()) {
            switch (read_choice("output", {"string", "symbols", "sequences"}, outputModeIter->second)) {
                case 0:
                    this->output_format = OutputFormat::String;
                    break;
                case 1:
                    this->output_format = OutputFormat::SymbolCell;
                    break;
                case 2:
                    this->output_format = OutputFormat::OperatorCell;
                    break;
                default:
                    throw_error(this->matlabEngine, errors::bad_param, "Unknown output mode.");
            }
        }

    }

    void ApplyMomentRules::extra_input_checks(ApplyMomentRulesParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    ApplyMomentRules::ApplyMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 1;

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
        const auto& symbols = matrixSystem.Symbols();

        // Retrieve rules, or throw
        const auto& rulebook = [&]() -> const MomentSubstitutionRulebook& {
            try {
                return matrixSystem.rulebook(input.rulebook_index); // <- throws, if not found.
            } catch (const Moment::errors::missing_component& mce) {
                std::stringstream errSS;
                errSS << "Could not find rulebook at index " << input.rulebook_index << ".";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
        }();
        const auto& factory = rulebook.Factory();

        // Convert input to polynomial.
        Polynomial polynomial = raw_data_to_polynomial(this->matlabEngine, factory, input.raw_polynomial);

        // Echo input in debug mode
        if (this->verbose) {
            std::stringstream debugSS;
            debugSS << "Input polynomial: " << polynomial << "\n";
            print_to_console(this->matlabEngine, debugSS.str());
        }

        bool match = rulebook.reduce_in_place(polynomial);

        std::string str_output;
        if (this->verbose || (input.output_format == ApplyMomentRulesParams::OutputFormat::String)) {
            std::stringstream transformedSS;
            transformedSS << polynomial;
            str_output = transformedSS.str();
        }

        if (this->verbose) {
            std::stringstream debugSS;
            debugSS << "Output polynomial ";
            if (!match) {
                debugSS << "unchanged.";
            } else {
                debugSS << ": " << str_output;
            }
            debugSS  << "\n";
            print_to_console(this->matlabEngine, debugSS.str());
        }

        if (input.output_format == ApplyMomentRulesParams::OutputFormat::String) {
            matlab::data::ArrayFactory mlfactory;
            output[0] = mlfactory.createScalar(str_output);
        } else if (input.output_format == ApplyMomentRulesParams::OutputFormat::SymbolCell) {
            PolynomialExporter polynomialExporter{this->matlabEngine, symbols};
            output[0] = polynomialExporter.direct(polynomial);
        } else if (input.output_format == ApplyMomentRulesParams::OutputFormat::OperatorCell) {
            PolynomialExporter polynomialExporter{this->matlabEngine, symbols};
            output[0] = polynomialExporter.sequences(polynomial);
        } else {
            throw_error(this->matlabEngine, errors::bad_param, "Unknown output format.");
        }
    }

}