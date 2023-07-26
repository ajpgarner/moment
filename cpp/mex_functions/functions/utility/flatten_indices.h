/**
 * flatten_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mex_function.h"

#include <vector>

namespace Moment::mex::functions  {

    class FlattenIndicesParams : public SortedInputs {
    public:
        /** Set to true to index from 0 (C style), or false to index from 1 (matlab style) */
        bool zero_index = false;

        /** The object whose indices we are flattening. */
        std::vector<size_t> dimensions;

        /** A list of indices per dimension. */
        std::vector<std::vector<size_t>> indices;

        explicit FlattenIndicesParams(SortedInputs&& input);
    };

    class FlattenIndices : public ParameterizedMexFunction<FlattenIndicesParams, MEXEntryPointID::FlattenIndices> {
    public:
        explicit FlattenIndices(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, FlattenIndicesParams &input) override;
    };
}
