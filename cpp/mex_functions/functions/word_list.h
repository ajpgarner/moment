/**
 * word_list.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../mex_function.h"

namespace Moment::mex::functions  {

    struct WordListParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        size_t word_length = 0;

    public:
        explicit WordListParams(SortedInputs&& inputs);
    };

    class WordList : public ParameterizedMexFunction<WordListParams, MEXEntryPointID::WordList> {
    public:
        explicit WordList(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, WordListParams &input) override;

        void extra_input_checks(WordListParams &input) const override;

    };

}
