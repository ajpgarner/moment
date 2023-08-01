/**
 * settings.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
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
        // Locality format param
        auto lf_iter = this->params.find(u"locality_format");
        if (lf_iter != this->params.end()) {
            this->change_lof = read_choice_lof(lf_iter->second);
            this->any_changes = true;
        }

        // Multithreading param
        auto mt_iter = this->params.find(u"multithreading");
        if (mt_iter != this->params.end()) {
            this->change_mt = read_choice_mt(mt_iter->second);
            this->any_changes = true;
        }

    }

    void SettingsParams::getFromStruct() {
        // Check input is a struct
        if (inputs[0].getType() != matlab::data::ArrayType::STRUCT) {
            throw_error(matlabEngine, errors::bad_param,
                        "Input to settings must be a struct. (Possible misspelled parameter supplied!)");
        }

        // Test parameters are not set
        if (this->params.contains(u"locality_format") || this->params.contains(u"multithreading")) {
            throw_error(matlabEngine, errors::bad_param,
                        "If structured input supplied, no settings parameters should be supplied.");
        }

        // Check struct dimensions
        const matlab::data::StructArray structInput = inputs[0];
        auto dims = structInput.getDimensions();
        if ((dims.size() != 2) || (dims[0] != 1) || (dims[1] != 1)) {
            throw_error(matlabEngine, errors::bad_param,
                        "Input struct array must contain only one row.");
        }


        // Read fields of input struct
        for (const auto& name : structInput.getFieldNames()) {
            std::string nameStr = static_cast<std::string>(name);
            if (nameStr == "locality_format") {
                this->change_lof = read_choice_lof(structInput[0][nameStr]);
                this->any_changes = true;
            } else if (nameStr == "multithreading") {
                this->change_mt = read_choice_mt(structInput[0][nameStr]);
                this->any_changes = true;
            } else {
                this->unknown_settings.emplace_back(nameStr);
            }
        }
    }


    SettingsParams::change_lof_t SettingsParams::read_choice_lof(const matlab::data::Array &field) const {
        try {
            auto choice = read_choice("locality_format", {"natural", "traditional"}, field);
            switch (choice) {
                case 0:
                    return change_lof_t::LOF_Natural;
                case 1:
                    return change_lof_t::LOF_Traditional;
                default: [[unlikely]]
                    break;
            }
        } catch (const errors::invalid_choice& ice) {
            throw_error(matlabEngine, errors::bad_param, ice.what());
        }
        throw_error(matlabEngine, errors::bad_param, "Unknown locality formatter choice.");
    }



    SettingsParams::change_mt_t SettingsParams::read_choice_mt(const matlab::data::Array &field) const {
        try {
            auto choice = read_choice("multithreading", {"off", "on", "auto", "always"}, field);
            switch (choice) {
                case 0:
                    return change_mt_t::MT_Off;
                case 1:
                case 2:
                    return change_mt_t::MT_Auto;
                case 3:
                    return change_mt_t::MT_Always;
                default: [[unlikely]]
                    break;
            }
        } catch (const errors::invalid_choice& ice) {
            throw_error(matlabEngine, errors::bad_param, ice.what());
        }
        throw_error(matlabEngine, errors::bad_param, "Unknown multithreading choice.");
    }

    Settings::Settings(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = 0;
        this->max_inputs = 1;
        this->flag_names.emplace(u"structured");
        this->param_names.emplace(u"locality_format");
        this->param_names.emplace(u"multithreading");
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

            // Change MT policy, if necessary
            switch (input.change_mt) {
                case SettingsParams::change_mt_t::MT_Off:
                    cloned_settings->set_mt_policy(Multithreading::MultiThreadPolicy::Never);
                    break;
                case SettingsParams::change_mt_t::MT_Auto:
                    cloned_settings->set_mt_policy(Multithreading::MultiThreadPolicy::Optional);
                    break;
                case SettingsParams::change_mt_t::MT_Always:
                    if (!this->quiet) {
                        print_warning(matlabEngine, "Due to thread-construction costs 'always' multithreading mode may be slower than 'auto' or 'off'.");
                    }
                    cloned_settings->set_mt_policy(Multithreading::MultiThreadPolicy::Always);
                    break;
                case SettingsParams::change_mt_t::MT_Unchanged:
                default:
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
        auto output = factory.createStructArray({1, 1}, {"locality_format", "multithreading"});
        output[0]["locality_format"] = factory.createScalar(vars.get_locality_formatter()->name());
        switch (vars.get_mt_policy()) {
            case Multithreading::MultiThreadPolicy::Never:
                output[0]["multithreading"] = factory.createScalar("off");
                break;
            case Multithreading::MultiThreadPolicy::Optional:
                output[0]["multithreading"] = factory.createScalar("auto");
                break;
            case Multithreading::MultiThreadPolicy::Always:
                output[0]["multithreading"] = factory.createScalar("always");
                break;
        }
        return output;
    }
}