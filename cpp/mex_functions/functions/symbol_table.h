/**
 * symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "integer_types.h"

namespace NPATK {
    class SymbolTable;
    class Context;
}

namespace NPATK::mex::functions  {

    struct SymbolTableParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        uint64_t from_id = 0;

        enum class OutputMode {
            AllSymbols,
            FromId,
            SearchBySequence
        } output_mode = OutputMode::AllSymbols;

        std::vector<oper_name_t> sequence;

    public:
        explicit SymbolTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);
    };

    class SymbolTable : public NPATK::mex::functions::MexFunction {
    public:
        explicit SymbolTable(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void find_and_return_symbol(IOArgumentRange output, const SymbolTableParams& input,
                                    const Context &context, const NPATK::SymbolTable &table);
    };

}
