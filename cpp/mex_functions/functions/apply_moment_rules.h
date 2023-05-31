/**
 * apply_moment_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mex_function.h"

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

        enum class OutputFormat {
            Cell,
            String
        } output_format = OutputFormat::Cell;

        /** Direct set of symbol combos, if specified. */
        std::vector<raw_sc_data> raw_polynomial;

    public:
        explicit ApplyMomentRulesParams(SortedInputs&& structuredInputs);

    };

    class ApplyMomentRules : public ParameterizedMexFunction<ApplyMomentRulesParams, MEXEntryPointID::ApplyMomentRules> {
    public:
        explicit ApplyMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ApplyMomentRulesParams &input) override;

        void extra_input_checks(ApplyMomentRulesParams &input) const override;
    };
}