/**
 * apply_moment_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mtk_function.h"

#include "integer_types.h"

#include "import/read_polynomial.h"

#include <vector>

namespace Moment::mex::functions {

    struct ApplyMomentRulesParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The reference to the substitution rules . */
        uint64_t rulebook_index = 0;

        /** The format in which the inputs polynomial is given. */
        enum class InputFormat {
            SymbolCell,
            OperatorCell
        } input_format = InputFormat::SymbolCell;

        /** The format in which the output polynomial is given. */
        enum class OutputFormat {
            SymbolCell,
            Polynomial,
            String
        } output_format = OutputFormat::SymbolCell;

        /** Direct set of symbol combos, if specified. */
        std::vector<std::vector<raw_sc_data>> raw_polynomial;

        /** Shape of input array. */
        std::vector<size_t> input_shape;

    public:
        explicit ApplyMomentRulesParams(SortedInputs&& structuredInputs);

        void read_symbol_cell_input(const matlab::data::Array& array);


    };

    class ApplyMomentRules : public ParameterizedMTKFunction<ApplyMomentRulesParams, MTKEntryPointID::ApplyMomentRules> {
    public:
        explicit ApplyMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ApplyMomentRulesParams &input) override;

        void extra_input_checks(ApplyMomentRulesParams &input) const override;
    };
}