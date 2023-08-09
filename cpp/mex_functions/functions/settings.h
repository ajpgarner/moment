/**
 * settings.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mtk_function.h"

#include <string>
#include <vector>

namespace Moment::mex::functions  {

    struct SettingsParams : public SortedInputs {
    public:
        bool structured_output = false;

        bool any_changes = false;

        enum class change_lof_t {
            LOF_Unchanged = 0,
            LOF_Natural,
            LOF_Traditional,
        } change_lof = change_lof_t::LOF_Unchanged;

        enum class change_mt_t {
            MT_Unchanged = 0,
            MT_Off,
            MT_Auto,
            MT_Always,
        } change_mt = change_mt_t::MT_Unchanged;

        std::vector<std::string> unknown_settings;

    public:
        explicit SettingsParams(SortedInputs&& inputs);

    private:
        void getFromParams();

        void getFromStruct();

        change_lof_t read_choice_lof(const matlab::data::Array& field) const;

        change_mt_t read_choice_mt(const matlab::data::Array& field) const;
    };

    class Settings : public ParameterizedMTKFunction<SettingsParams, MTKEntryPointID::Settings> {
    public:
        explicit Settings(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void validate_output_count(size_t outputs, const SortedInputs &inputs) const override;

    protected:
        void operator()(IOArgumentRange output, SettingsParams &input) override;

        void extra_input_checks(SettingsParams &input) const override;

    private:
        matlab::data::StructArray make_settings_struct(const EnvironmentalVariables& vars) const;
    };

}
