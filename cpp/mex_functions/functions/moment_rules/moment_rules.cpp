/**
 * moment_rules.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_rules.h"

#include "storage_manager.h"

#include "scenarios/context.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/rules/moment_rulebook.h"
#include "symbolic/symbol_table.h"

#include "utilities/read_as_scalar.h"
#include "utilities/read_as_string.h"
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/read_choice.h"

#include "export/export_moment_substitution_rules.h"

namespace Moment::mex::functions {

    MomentRulesParams::MomentRulesParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {
        // Read matrix key
        this->matrix_system_key = read_positive_integer<size_t>(matlabEngine, "Matrix system reference", inputs[0], 0);

        // Read rulebook index
        this->rulebook_index = read_positive_integer<uint64_t>(matlabEngine, "Rulebook index", this->inputs[1], 0);

        // Ascertain output mode.
        if (this->inputs.size() >= 3) {
            try {
                switch (read_choice("Output mode",
                                    {"strings", "symbols", "polynomials", "rewrite", "homogenous"},
                                    this->inputs[2])) {
                    case 0:
                        this->output_mode = OutputMode::String;
                        break;
                    case 1:
                        this->output_mode = OutputMode::SymbolCell;
                        break;
                    case 2:
                        this->output_mode = OutputMode::Polynomial;
                        break;
                    case 3:
                        this->output_mode = OutputMode::RewriteMatrix;
                        break;
                    case 4:
                        this->output_mode = OutputMode::HomogenousMatrix;
                        break;
                }
            } catch (const Moment::mex::errors::invalid_choice& ice) {
                throw_error(this->matlabEngine, errors::bad_param, ice.what());
            }
        }
    }


     MomentRules::MomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 3;

        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void MomentRules::extra_input_checks(MomentRulesParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }

    void MomentRules::operator()(IOArgumentRange output, MomentRulesParams &input) {
        // Get stored matrix system
        auto msPtr = [&]() -> std::shared_ptr<MatrixSystem> {
            try {
                return this->storageManager.MatrixSystems.get(input.matrix_system_key);
            } catch (const Moment::errors::not_found_error& nfe) {
                throw_error(this->matlabEngine, errors::bad_param,
                            std::string("Matrix system not found: ").append(nfe.what()));
            }
        }();
        auto& system = *msPtr;

        // Acquire read-lock on matrix system.
        auto new_read_lock = msPtr->get_read_lock();

        // Try to get rulebook
        const auto& rulebook = [&]() -> const MomentRulebook& {
           try {
               return msPtr->rulebook(input.rulebook_index);
           } catch (const Moment::errors::missing_component& mce) {
               throw_error(this->matlabEngine, errors::bad_param, mce.what());
           }
        }();


        // Extra info, if verbose
        if (this->verbose) {
            std::stringstream infoSS;
            infoSS << "Rulebook #" << input.rulebook_index << ": " << rulebook.name() << "\n";
            const size_t rb_size = rulebook.size();
            infoSS << "Contains " << rb_size << ((rb_size != 1) ? " rules" : " rule") << ".\n";
            if (rulebook.is_hermitian()) {
                infoSS << "Is hermitian-preserving.\n";
            } else {
                infoSS << "Is not hermitian-preserving.\n";
            }
            if (rulebook.is_monomial()) {
                infoSS << "Is monomial-preserving.\n";
            } else {
                infoSS << "Is not monomial-preserving.\n";
            }
            print_to_console(this->matlabEngine, infoSS.str());
        }

        // How do we output?
        matlab::data::ArrayFactory factory;
        MomentSubstitutionRuleExporter msrExporter{this->matlabEngine, system.Symbols(),
                                                   system.polynomial_factory().zero_tolerance};

        switch (input.output_mode) {
            case MomentRulesParams::OutputMode::String:
                output[0] = msrExporter.as_string(rulebook);
                break;
            case MomentRulesParams::OutputMode::SymbolCell:
                output[0] = msrExporter.as_symbol_cell(rulebook);
                break;
            case MomentRulesParams::OutputMode::Polynomial:
                output[0] = msrExporter.as_polynomials(rulebook);
                break;
            case MomentRulesParams::OutputMode::RewriteMatrix:
                output[0] = msrExporter.as_rewrite_matrix(rulebook);
                break;
            case MomentRulesParams::OutputMode::HomogenousMatrix:
                output[0] = msrExporter.as_homogenous_matrix(rulebook);
                break;
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown output mode!");
        }
    }
}