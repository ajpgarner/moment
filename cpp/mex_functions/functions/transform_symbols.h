/**
 * transform_symbols.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../mex_function.h"

#include "integer_types.h"

#include <vector>


namespace Moment::mex::functions {

    struct TransformSymbolsParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        enum class InputType {
            Unknown,
            SymbolId,
            Basis
        } input_type = InputType::Unknown;

        enum class OutputType {
            Unknown,
            String,
            Basis
        } output_type = OutputType::Unknown;

        symbol_name_t symbol_id;

    public:
        explicit TransformSymbolsParams(SortedInputs&& structuredInputs);

    };

    class TransformSymbols : public ParameterizedMexFunction<TransformSymbolsParams, MEXEntryPointID::TransformSymbols> {
    public:
        explicit TransformSymbols(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage);

    protected:
        void operator()(IOArgumentRange output, TransformSymbolsParams &input) override;

        void extra_input_checks(TransformSymbolsParams &input) const override;

    };
}