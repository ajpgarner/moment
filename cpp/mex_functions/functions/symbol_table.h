/**
 * symbol_table.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mtk_function.h"
#include "integer_types.h"

namespace Moment {
    class MatrixSystem;
}

namespace Moment::mex {
    class SymbolTableExporter;
}

namespace Moment::mex::functions  {

    struct SymbolTableParams : public SortedInputs {
    public:
        /** The associated matrix system ID */
        uint64_t storage_key = 0;

        /** How should the table be exported? */
        enum class OutputMode {
            AllSymbols,
            FromId,
            SearchBySequence,
            SearchBySequenceArray,
        } output_mode = OutputMode::AllSymbols;

        /** The first symbol to be included in the output. */
        uint64_t from_id = 0;

        /** The flattened list of sequences. */
        std::vector<std::vector<oper_name_t>> sequences;

        /** The dimensions of the (cell) array the input sequences are provided in. */
        std::vector<size_t> sequence_dimensions;

    public:
        explicit SymbolTableParams(SortedInputs&& inputs);

        [[nodiscard]] std::string to_string() const override;
    };

    class SymbolTable : public ParameterizedMTKFunction<SymbolTableParams, MTKEntryPointID::SymbolTable> {
    public:
        explicit SymbolTable(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, SymbolTableParams& input) final;

    protected:
        void extra_input_checks(SymbolTableParams &input) const override;

    private:
        [[nodiscard]] matlab::data::Array
        find_and_return_symbol(const SymbolTableParams& input, const SymbolTableExporter& exporter);

        [[nodiscard]] matlab::data::Array
        find_and_return_symbol_array(const SymbolTableParams& input, const SymbolTableExporter& exporter);
    };

}
