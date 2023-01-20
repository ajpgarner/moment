/**
 * release.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "storage_manager.h"

namespace Moment::mex::functions {

    class ReleaseParams : public SortedInputs {
    public:
        enum class StorableType {
            Unknown = 0,
            MatrixSystem
        } type = StorableType::Unknown;
        size_t key = 0;

        ReleaseParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& raw_inputs);
    };

    class Release : public ParameterizedMexFunction<ReleaseParams, MEXEntryPointID::Release> {
    public:
        explicit Release(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ReleaseParams &input) override;

        void extra_input_checks(ReleaseParams &input) const override;

    };

}