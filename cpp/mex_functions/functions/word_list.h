/**
 * word_list.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../mtk_function.h"
#include "import/matrix_system_id.h"

namespace Moment::mex::functions  {

    struct WordListParams : public SortedInputs {
    public:
        /** Key to the matrix system whose words we want. */
        MatrixSystemId matrix_system_key;

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

    };

}
