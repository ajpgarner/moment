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

    SettingsParams::SettingsParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&raw_inputs)
        : SortedInputs(std::move(raw_inputs)) {
        auto lf_iter = this->params.find(u"locality_format");
        if (lf_iter != this->params.end()) {
            auto choice = read_choice("locality_format", {"natural", "traditional"}, lf_iter->second);
            switch (choice) {
                case 0:
                    this->change_lof = change_lof_t::LOF_Natural;
                    break;
                case 1:
                    this->change_lof = change_lof_t::LOF_Traditional;
                    break;
            }
            this->any_changes = true;
        } else {
            this->change_lof = change_lof_t::LOF_Unchanged;
        }
    }

    Settings::Settings(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction{matlabEngine, storage, u"settings"} {
        this->min_inputs = this->max_inputs = 0;
        this->param_names.emplace(u"locality_format");
        this->min_outputs = 0;
        this->max_outputs = 1;
    }

    void Settings::operator()(IOArgumentRange output, SettingsParams &input) {
        bool output_settings = (output.size() > 0) || !input.any_changes || this->verbose;

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

    void Settings::extra_input_checks(SettingsParams &input) const {
        ParameterizedMexFunction::extra_input_checks(input);
    }

}