/**
 * get_symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions  {

    struct GetSymbolTableParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        uint64_t from_id = 0;

        enum class OutputMode {
            AllSymbols,
            FromId
        } output_mode = OutputMode::AllSymbols;

    public:
        explicit GetSymbolTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

        friend class GetSymbolTable;
    };

    class GetSymbolTable : public NPATK::mex::functions::MexFunction {
    public:
        explicit GetSymbolTable(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}
