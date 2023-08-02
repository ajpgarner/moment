/**
 * list.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"

namespace Moment::mex::functions  {

    struct ListParams : public SortedInputs {
    public:
        enum class OutputType {
            All,
            OneSystem
        } output_type = OutputType::All;

        uint64_t matrix_system_key = 0;

        bool structured = false;

        bool export_symbols = false;

        bool export_matrix_properties = false;

    public:
        explicit ListParams(SortedInputs&& inputs);

    };

    class List : public ParameterizedMexFunction<ListParams, MEXEntryPointID::List> {
    public:
        explicit List(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ListParams &input) override;

        void extra_input_checks(ListParams &input) const override;

    private:
        std::string generateListString(const ListParams &input) const;

        matlab::data::StructArray generateListStruct() const;

        matlab::data::StructArray generateOneSystemStruct(const ListParams &input) const;
    };

}
