/**
 * release.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "storage_manager.h"

namespace NPATK::mex::functions {

    class ReleaseParams : public SortedInputs {
    public:
        enum class StorableType {
            Unknown = 0,
            MatrixSystem
        } type = StorableType::Unknown;
        size_t key = 0;

        ReleaseParams(matlab::engine::MATLABEngine &matlabEngine, const StorageManager& storage,
                      SortedInputs&& raw_inputs);
    };

    class Release : public MexFunction {
    public:
        explicit Release(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final {
            return std::make_unique<ReleaseParams>(this->matlabEngine, this->storageManager, std::move(*input));
        }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };

}