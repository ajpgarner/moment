/**
 * collins_gisin.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mex_function.h"

namespace Moment::mex::functions  {

    struct CollinsGisinParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        enum class OutputType {
            Sequences,
            SymbolIds,
            SequenceStrings
        } outputType = OutputType::Sequences;

    public:
        explicit CollinsGisinParams(SortedInputs&& inputs);

    };

    class CollinsGisin : public ParameterizedMexFunction<CollinsGisinParams, MEXEntryPointID::CollinsGisin> {
    public:
        explicit CollinsGisin(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CollinsGisinParams &input) override;

        void extra_input_checks(CollinsGisinParams &input) const override;

    };

}
