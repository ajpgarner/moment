/**
 * settings.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "settings.h"

#include "scenarios/locality/locality_operator_formatter.h"

#include "storage_manager.h"
#include "utilities/read_choice.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    SettingsParams::SettingsParams(SortedInputs &&raw_inputs)
        : SortedInputs(std::move(raw_inputs)) {

        if (!inputs.empty()) {
            this->getFromStruct();
        } else {
            this->getFromParams();
        }

        this->structured_output = this->flags.contains(u"structured");
    }


    void SettingsParams::getFromParams() {
        auto lf_iter = this->params.find(u"locality_format");
        if (lf_iter != this->params.end()) {
            try {
                auto choice = read_choice("locality_format", {"natural", "traditional"}, lf_iter->second);
                switch (choice) {
                    case 0:
                        this->change_lof = change_lof_t::LOF_Natural;
                        break;
                    case 1:
                        this->change_lof = change_lof_t::LOF_Traditional;
                        break;
                    default:
                        assert(false);
                        break;
                }
                this->any_changes = true;
            } catch (const errors::invalid_choice& ice) {
                throw_error(matlabEngine, errors::bad_param, ice.what());
            }
        } else {
            this->change_lof = change_lof_t::LOF_Unchanged;
        }
    }

    void SettingsParams::getFromStruct() {
        if (inputs[0].getType() != matlab::data::ArrayType::STRUCT) {
            throw_error(matlabEngine, errors::bad_param,
                        "Input to settings must be a struct. (Possible misspelled parameter supplied!)");
        }

        const matlab::data::StructArray structInput = inputs[0];
        auto dims = structInput.getDimensions();
        if ((dims.size() != 2) || (dims[0] != 1) || (dims[1] != 1)) {
            throw_error(matlabEngine, errors::bad_param,
                        "Input struct array must contain only one row.");
        }

        // Now, test params not also set
        if (this->params.contains(u"locality_format")) {
            throw_error(matlabEngine, errors::bad_param,
                        "If structured input supplied, no settings parameters should be supplied.");
        }

        for (const auto& name : structInput.getFieldNames()) {
            std::string nameStr = static_cast<std::string>(name);
            if (nameStr == "locality_format") {
                auto choice = read_choice("locality_format", {"natural", "traditional"}, structInput[0][nameStr]);
                switch (choice) {
                    case 0:
                        this->change_lof = change_lof_t::LOF_Natural;
                        break;
                    case 1:
                        this->change_lof = change_lof_t::LOF_Traditional;
                        break;
                    default:
                        assert(false);
                        break;
                }
                this->any_changes = true;
            } else {
                this->unknown_settings.emplace_back(nameStr);
            }
        }
    }

    Settings::Settings(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction{matlabEngine, storage, u"settings"} {
        this->min_inputs = 0;
        this->max_inputs = 1;
        this->flag_names.emplace(u"structured");
        this->param_names.emplace(u"locality_format");
        this->min_outputs = 0;
        this->max_outputs = 1;
    }

    void Settings::validate_output_count(size_t outputs, const SortedInputs &inputRaw) const {
        const auto& inputs = dynamic_cast<const SettingsParams&>(inputRaw);
        if (inputs.structured_output && (outputs != 1)) {
            throw_error(this->matlabEngine, errors::too_few_outputs, "Structured output mode requires one output.");
        }
    }

    void Settings::operator()(IOArgumentRange output, SettingsParams &input) {
        bool output_settings = (output.size() > 0) || !input.any_changes || this->verbose;

        if (!this->quiet && !input.unknown_settings.empty()) {
            std::stringstream ss;
            ss << "WARNING: The following settings fields were not understood: ";
            bool done_one = false;
            for (const auto& us : input.unknown_settings) {
                if (done_one) {
                    ss << ", ";
                } else {
                    done_one = true;
                }
                ss << "\"" << us << "\"";
            }
            ss << ".\n";
            print_to_console(matlabEngine, ss.str());
        }

        std::shared_ptr<const EnvironmentalVariables> altered_settings;
        if (input.any_changes) {
            std::shared_ptr<EnvironmentalVariables> cloned_settings = std::make_unique<EnvironmentalVariables>(*this->settings);

            // Change LOF, if necessary
            switch (input.change_lof) {
                case SettingsParams::change_lof_t::LOF_Natural:
                    cloned_settings->set_locality_formatter(std::make_shared<Locality::NaturalLOFormatter>());
                    break;
                case SettingsParams::change_lof_t::LOF_Traditional:
                    cloned_settings->set_locality_formatter(std::make_shared<Locality::TraditionalLOFormatter>());
                    break;
                default:
                case SettingsParams::change_lof_t::LOF_Unchanged:
                    break;
            }

            // Save new settings
            this->storageManager.Settings.set(cloned_settings);
            altered_settings = std::const_pointer_cast<const EnvironmentalVariables>(cloned_settings);
        } else {
            altered_settings = this->settings;
        }

        // Write setting summary
        if (output_settings) {
            if (input.structured_output) {
                output[0] = this->make_settings_struct(*altered_settings);
            } else {
                matlab::data::ArrayFactory factory;
                std::stringstream ss;
                ss << *altered_settings;
                if (output.size() > 0) {
                    output[0] = factory.createScalar(ss.str());
                }
                if ((output.size() <= 0) || (this->verbose)) {
                    ss << "\n";
                    print_to_console(this->matlabEngine, ss.str());
                }
            }
        }
    }

    void Settings::extra_input_checks(SettingsParams &input) const {
        ParameterizedMexFunction::extra_input_checks(input);
    }

    matlab::data::StructArray Settings::make_settings_struct(const EnvironmentalVariables& vars) const {
        matlab::data::ArrayFactory factory;
        auto output = factory.createStructArray({1, 1}, {"locality_format"});
        output[0]["locality_format"] = factory.createScalar(vars.get_locality_formatter()->name());
        return output;
    }
}