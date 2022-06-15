/**
 * make_moment_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_moment_matrix.h"

#include "operators/context.h"
#include "operators/moment_matrix.h"

#include "fragments/parse_to_context.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include <memory>

namespace NPATK::mex::functions {

    namespace {
        std::unique_ptr<Context> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                              const MakeMomentMatrixParams& input) {
            switch (input.specification_mode) {
                case MakeMomentMatrixParams::SpecificationMode::FlatNoMeasurements:
                    return std::make_unique<Context>(PartyInfo::MakeList(input.number_of_parties,
                                                     input.flat_operators_per_party));
                    break;
                case MakeMomentMatrixParams::SpecificationMode::FlatWithMeasurements:
                    return std::make_unique<Context>(PartyInfo::MakeList(input.number_of_parties,
                                                     input.flat_mmts_per_party, input.flat_outcomes_per_mmt));
                    break;
                case MakeMomentMatrixParams::SpecificationMode::FromSettingObject:
                    assert(input.ptrSettings != nullptr);
                    return parse_to_context(matlabEngine, *input.ptrSettings);
                default:
                case MakeMomentMatrixParams::SpecificationMode::Unknown:
                    throw_error(matlabEngine, "not_implemented", "Unknown input format!");
            }
            assert(false);
        }
    }

    MakeMomentMatrix::MakeMomentMatrix(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeMomentMatrix, u"make_moment_matrix") {
        this->min_outputs = 1;
        this->max_outputs = 2;

        this->flag_names.emplace(u"sequences");

        this->param_names.emplace(u"setting");

        this->param_names.emplace(u"parties");
        this->param_names.emplace(u"measurements");
        this->param_names.emplace(u"outcomes");
        this->param_names.emplace(u"operators");

        this->param_names.emplace(u"level");

        this->mutex_params.add_mutex(u"outcomes", u"operators");

        this->mutex_params.add_mutex(u"setting", u"parties");
        this->mutex_params.add_mutex(u"setting", u"measurements");
        this->mutex_params.add_mutex(u"setting", u"outcomes");
        this->mutex_params.add_mutex(u"setting", u"operators");

        this->min_inputs = 0;
        this->max_inputs = 4;
    }

    MakeMomentMatrixParams::MakeMomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {
        this->output_sequences = this->flags.contains(u"sequences");

        // Either set named params OR give multiple params
        bool setting_specified = this->params.contains(u"setting");
        bool set_any_flat_param = this->params.contains(u"parties")
                             || this->params.contains(u"measurements")
                             || this->params.contains(u"operators");

        bool set_any_param = setting_specified || set_any_flat_param || this->params.contains(u"level");
        assert(!(setting_specified && set_any_flat_param)); // mutex should rule out.

        if (set_any_param) {
            // No extra inputs
            if (!inputs.empty()) {
                throw errors::BadInput{errors::bad_param,
                                       "Input arguments should be exclusively named, or exclusively unnamed."};
            }

            // Read and check level - required.
            auto &depth_param = this->find_or_throw(u"level");
            this->hierarchy_level = read_positive_integer(matlabEngine, "Parameter 'level'", depth_param, 0);

            if (setting_specified) {
                this->verifyAsContext(matlabEngine, this->params[u"setting"]);
                this->ptrSettings = &(this->params[u"setting"]);
                return;
            }
            if (set_any_flat_param) {
                this->getFlatFromParams(matlabEngine);
                return;
            }
            assert(false);
        }

        // No named parameters... try to interpret inputs as Settings object + depth
        if (this->inputs.size() == 2) {
            if ((this->inputs[0].getType() == matlab::data::ArrayType::OBJECT)
                || (this->inputs[0].getType() == matlab::data::ArrayType::HANDLE_OBJECT_REF)) {
                this->verifyAsContext(matlabEngine, this->inputs[0]);
                this->ptrSettings = &(this->inputs[0]);

                // Read and check level
                this->hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level",
                                                        inputs[inputs.size() - 1], 0);
                return;
            }
        }

        // Otherwise, try to interpret inputs as flat specification
        this->getFlatFromInputs(matlabEngine);

    }

    void MakeMomentMatrixParams::getFlatFromParams(matlab::engine::MATLABEngine &matlabEngine) {
        // Read and check number of parties, or default to 1
        auto party_param = params.find(u"parties");
        if (party_param != params.end()) {
            bool has_opers = params.contains(u"operators");
            bool has_mmts = params.contains(u"measurements");
            if (!(has_opers || has_mmts)) {
                throw errors::BadInput{errors::missing_param,
                                       "If 'parties' is set, then one of 'operators' or 'measurements' must also be set."};
            }
            number_of_parties = read_positive_integer(matlabEngine, "Parameter 'parties'",
                                                      party_param->second, 1);
        } else {
            number_of_parties = 1;
        }

        // Read and check measurements
        auto mmt_param = params.find(u"measurements");
        if (mmt_param != params.end()) {
            specification_mode = SpecificationMode::FlatWithMeasurements;
            flat_mmts_per_party = read_positive_integer(matlabEngine, "Parameter 'measurements'",
                                                        mmt_param->second, 1);
        } else {
            specification_mode = SpecificationMode::FlatNoMeasurements;
            flat_mmts_per_party = 0;
        }

        // Number of operators must also always be specified
        if (specification_mode == SpecificationMode::FlatWithMeasurements) {
            auto outcome_param = params.find(u"outcomes");
            if (outcome_param == params.end()) {
                throw errors::BadInput{errors::missing_param,
                                       "Parameter 'outcomes' must be set, if 'measurements' is also set."};
            }
            flat_outcomes_per_mmt = read_positive_integer(matlabEngine, "Parameter 'outcomes'",
                                                          outcome_param->second, 1);
        } else if (specification_mode == SpecificationMode::FlatNoMeasurements) {
            auto oper_param = params.find(u"operators");
            if (oper_param == params.end()) {
                throw errors::BadInput{errors::missing_param,
                                       "Parameter 'operators' must be set, if 'measurements' is not set."};
            }
            if (!castable_to_scalar_int(oper_param->second)) {
                throw errors::BadInput{errors::missing_param,
                                       "Parameter 'operators' must be a positive scalar integer."};
            }
            flat_operators_per_party = read_positive_integer(matlabEngine, "Parameter 'operators'",
                                                             oper_param->second, 1);
        }
    }


    void MakeMomentMatrixParams::getFlatFromInputs(matlab::engine::MATLABEngine &matlabEngine) {
        if (inputs.size() < 2) {
            std::string errStr{"Please supply either named inputs; or a list of integers in the"};
            errStr += " form of [operators, level], ";
            errStr += "[parties, operators per party, level], ";
            errStr += "or [parties, measurements per party, outcomes per measurement].";
            throw errors::BadInput{errors::too_few_inputs, errStr};
        }

        // Read depth
        hierarchy_level = read_positive_integer(matlabEngine, "Hierarchy level",
                                                inputs[inputs.size() - 1], 0);

        // Work out where the operator count should be
        size_t operator_index = 0;
        if (inputs.size() == 2) {
            // Operator_index stays as 0: op, depth
            specification_mode = SpecificationMode::FlatNoMeasurements;
        } else if (inputs.size() == 3) {
            operator_index = 1; // party, op, depth
            specification_mode = SpecificationMode::FlatNoMeasurements;
        } else if (inputs.size() == 4) {
            operator_index = 2; // party, mmts, ops, depth
            specification_mode = SpecificationMode::FlatWithMeasurements;
        }

        // Read measurements (if any) and operator count
        if (specification_mode == SpecificationMode::FlatWithMeasurements) {
            flat_mmts_per_party =  read_positive_integer(matlabEngine, "Measurement count",
                                                         inputs[1], 1);
            flat_outcomes_per_mmt = read_positive_integer(matlabEngine, "Number of outcomes",
                                                          inputs[operator_index], 1);
        } else {
            flat_mmts_per_party = 0;
            flat_operators_per_party = read_positive_integer(matlabEngine, "Number of operators",
                                                             inputs[operator_index], 1);
        }
    }


    void MakeMomentMatrixParams::verifyAsContext(matlab::engine::MATLABEngine &matlabEngine,
                                                 const matlab::data::Array &input) {
        auto [good, errMsg] = verify_as_setting(matlabEngine, input);
        if (!good) {
            throw errors::BadInput(errors::bad_param,
                                   std::string("Invalid setting: ") + errMsg.value());
        } else {
            this->specification_mode = SpecificationMode::FromSettingObject;
        }
    }

    std::string MakeMomentMatrixParams::to_string() const {
        std::stringstream ss;
        switch (this->specification_mode) {
            case SpecificationMode::FlatNoMeasurements:
                ss << "Specified as parties with the same number of arbitrary operators.\n";
                break;
            case SpecificationMode::FlatWithMeasurements:
                ss << "Specified as parties with the same number of measurements / outcomes.\n";
                break;
            case SpecificationMode::FromSettingObject:
                ss << "Specified as a Setting object.\n";
                break;
            default:
            case SpecificationMode::Unknown:
                ss << "Unknown specification mode.\n";
                break;
        }

        switch (this->specification_mode) {
            case SpecificationMode::FlatNoMeasurements:
                ss << "Parties: " << this->number_of_parties << "\n";
                ss << "Operators per party: " << this->flat_operators_per_party << "\n";
                break;
            case SpecificationMode::FlatWithMeasurements:
                ss << "Parties: " << this->number_of_parties << "\n";
                ss << "Measurements per party: " << this->flat_mmts_per_party << "\n";
                ss << "Outcomes per measurement: " << this->flat_outcomes_per_mmt << "\n";
                break;
            case SpecificationMode::FromSettingObject:
                if (this->ptrSettings) {
                    ss << "Pointer to Setting object set.\n";
                } else {
                    ss << "Pointer to Setting object not set!\n";
                }
                break;
            default:
            case SpecificationMode::Unknown:
                break;
        }

        ss << "Hierarchy depth: " << this->hierarchy_level << "\n";
        return ss.str();
    }

    std::unique_ptr<SortedInputs>
    MakeMomentMatrix::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        return std::make_unique<MakeMomentMatrixParams>(this->matlabEngine, std::move(input));
    }

    void MakeMomentMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<MakeMomentMatrixParams&>(*inputPtr);

        auto contextPtr = make_context(this->matlabEngine, input);
        if (!contextPtr) {
            throw_error(this->matlabEngine, errors::internal_error, "Context object could not be created.");
        }

        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }


        throw_error(this->matlabEngine, "not_implemented", u"Not implemented");
    }




}