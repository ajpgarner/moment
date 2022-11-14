/**
 * complete.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "mex_function.h"
#include "integer_types.h"

#include "fragments/read_monomial_rules.h"

namespace NPATK::mex::functions {

    class CompleteParams : public SortedInputs {
    public:
        uint64_t max_operators = 0;

        uint64_t max_attempts = 128;

        bool hermitian_operators = true;

        bool commutative = false;

        /** True if testing for completion, without actually doing completion... */
        bool test_only = false;

        /** How the input to the complete command is supplied */
        enum class InputMode {
            FromCellArray,
            FromMatrixSystemID
        } input_mode = InputMode::FromCellArray;

        /** The raw rules (if provided...) */
        std::vector<RawMonomialRule> rules{};

        CompleteParams(matlab::engine::MATLABEngine &matlabEngine,
                       SortedInputs &&rawInput);

    };

    class Complete : public NPATK::mex::functions::MexFunction {
    public:
        explicit Complete(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}