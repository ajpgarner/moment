/**
 * settings.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "../mex_function.h"

namespace Moment::mex::functions  {

    struct SettingsParams : public SortedInputs {
    public:
        bool any_changes = false;

        enum class change_lof_t {
            LOF_Unchanged = 0,
            LOF_Natural,
            LOF_Traditional,
        } change_lof = change_lof_t::LOF_Unchanged;

    public:
        explicit SettingsParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

    };

    class Settings : public ParameterizedMexFunction<SettingsParams, MEXEntryPointID::Settings> {
    public:
        explicit Settings(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, SettingsParams &input) override;

        void extra_input_checks(SettingsParams &input) const override;

    };

}
