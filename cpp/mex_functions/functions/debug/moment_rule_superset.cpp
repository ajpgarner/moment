/**
 * moment_rule_superset.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_rule_superset.h"

#include "symbolic/rules/moment_rulebook.h"

#include "storage_manager.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"


namespace Moment::mex::functions  {

    MomentRuleSupersetParams::MomentRuleSupersetParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {
        this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                            this->inputs[0], 0);

        this->ruleset_A_index = read_positive_integer<uint64_t>(matlabEngine, "Rulebook A",
                                                            this->inputs[1], 0);

        this->ruleset_B_index = read_positive_integer<uint64_t>(matlabEngine, "Rulebook B",
                                                            this->inputs[2], 0);
    }

    MomentRuleSuperset::MomentRuleSuperset(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_outputs = this->max_outputs = 1;
        this->min_inputs = this->max_inputs = 3;
    }

    void MomentRuleSuperset::extra_input_checks(MomentRuleSupersetParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
    }

    void MomentRuleSuperset::operator()(IOArgumentRange output, MomentRuleSupersetParams &input) {
        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const Moment::errors::persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }
        const auto& matrixSystem = *matrixSystemPtr;

        // Get read lock on system
        std::shared_lock lock = matrixSystem.get_read_lock();

        // Get two referred-to rulebooks
        const auto& rulebook_A = [&]() -> const MomentRulebook& {
            try { return matrixSystem.Rulebook(input.ruleset_A_index); }
            catch (const Moment::errors::missing_component& mce) {
                throw_error(this->matlabEngine, errors::bad_param, "Rulebook A not found.");
            }
        }();
        const auto& rulebook_B = [&]() -> const MomentRulebook& {
            try { return matrixSystem.Rulebook(input.ruleset_B_index); }
            catch (const Moment::errors::missing_component& mce) {
                throw_error(this->matlabEngine, errors::bad_param, "Rulebook B not found.");
            }
        }();

        // Now, attempt to reduce every polynomial in rulebook_B
        const auto [result, in_A_not_B, in_B_not_A] = rulebook_A.compare_rulebooks(rulebook_B);

        const std::string str_result = [&](MomentRulebook::RulebookComparisonResult the_result) -> std::string {
            std::stringstream makeRS;

            makeRS << rulebook_A.name();
            switch (the_result) {
                case MomentRulebook::RulebookComparisonResult::AEqualsB:
                    makeRS << " == ";
                    break;
                case MomentRulebook::RulebookComparisonResult::AContainsB:
                    makeRS << " > ";
                    break;
                case MomentRulebook::RulebookComparisonResult::BContainsA:
                    makeRS << " < ";
                    break;
                case MomentRulebook::RulebookComparisonResult::Disjoint:
                    makeRS << " != ";
                    break;
                default:
                    makeRS << "???";
                    break;
            }
            makeRS << rulebook_B.name();
            return makeRS.str();
        }(result);

        if (this->verbose) {
            std::stringstream ss;
            ss << str_result << "\n";
            if (in_A_not_B != nullptr) {
                ss << "Only in " << rulebook_A.name() << ": " << in_A_not_B->as_polynomial(rulebook_A.factory) << "\n";
            }
            if (in_B_not_A != nullptr) {
                ss << "Only in " << rulebook_B.name() << ": " << in_B_not_A->as_polynomial(rulebook_B.factory) << "\n";
            }
            print_to_console(this->matlabEngine, ss.str());
        }

        if (output.size() > 0) {
            matlab::data::ArrayFactory factory;
            output[0] = factory.createScalar(str_result);
        }
    }

}