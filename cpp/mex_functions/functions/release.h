/**
 * release.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mtk_function.h"
#include "storage_manager.h"

namespace Moment::mex::functions {

    class ReleaseParams : public SortedInputs {
    public:
        enum class StorableType {
            Unknown = 0,
            MatrixSystem
        } type = StorableType::Unknown;
        uint64_t key = 0;

        explicit ReleaseParams(SortedInputs&& raw_inputs);
    };

    class Release : public ParameterizedMTKFunction<ReleaseParams, MTKEntryPointID::Release> {
    public:
        explicit Release(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ReleaseParams &input) override;

        void extra_input_checks(ReleaseParams &input) const override;

    };

}