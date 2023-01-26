/**
 * symbol_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../mex_function.h"
#include "integer_types.h"

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex::functions  {

    struct SymbolTableParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        uint64_t from_id = 0;

        enum class OutputMode {
            AllSymbols,
            FromId,
            SearchBySequence,
            SearchBySequenceArray,
        } output_mode = OutputMode::AllSymbols;

        std::vector<std::vector<oper_name_t>> sequences;

    public:
        explicit SymbolTableParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

        [[nodiscard]] std::string to_string() const override;
    };

    class SymbolTable : public ParameterizedMexFunction<SymbolTableParams, MEXEntryPointID::SymbolTable> {
    public:
        void validate_output_count(size_t outputs, const SortedInputs &inputs) const override;

        explicit SymbolTable(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, SymbolTableParams& input) final;

    protected:
        void extra_input_checks(SymbolTableParams &input) const override;

    private:
        void find_and_return_symbol(IOArgumentRange output, const SymbolTableParams& input,
                                    const MatrixSystem& system);

        void find_and_return_symbol_array(IOArgumentRange output, const SymbolTableParams& input,
                                    const MatrixSystem& system);
    };

}
