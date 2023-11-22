/**
 * word_list.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../mtk_function.h"

namespace Moment::mex::functions  {

    struct WordListParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        bool register_symbols = false;

        size_t word_length = 0;

        struct extra_data_t {
            size_t nearest_neighbours = 0;
        } extra_data;

        enum class OutputType {
            OperatorCell,
            Monomial,
            FullMonomial
        } output_type = OutputType::OperatorCell;

    public:
        explicit WordListParams(SortedInputs&& inputs);
    };

    class WordList : public ParameterizedMTKFunction<WordListParams, MTKEntryPointID::WordList> {
    public:
        explicit WordList(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, WordListParams &input) override;

        void extra_input_checks(WordListParams &input) const override;

    };

}
