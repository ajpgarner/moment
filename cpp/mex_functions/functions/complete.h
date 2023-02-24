/**
 * complete.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mex_function.h"
#include "integer_types.h"

#include "import/read_monomial_rules.h"

#include <memory>

namespace Moment::Algebraic {
    class NameTable;
}

namespace Moment::mex::functions {

    class CompleteParams : public SortedInputs {
    public:
        uint64_t max_operators = 0;

        uint64_t max_attempts = 128;

        bool hermitian_operators = true;

        bool normal_operators = true;

        bool commutative = false;

        /** True if testing for completion, without actually doing completion... */
        bool test_only = false;

        /** Name table object, for parsing rules */
        std::unique_ptr<Algebraic::NameTable> names;

        /** How the input to the complete command is supplied */
        enum class InputMode {
            FromCellArray,
            FromMatrixSystemID
        } input_mode = InputMode::FromCellArray;

        /** The raw rules (if provided...) */
        std::vector<RawMonomialRule> rules{};

    public:
        /** Constructor */
        explicit CompleteParams(SortedInputs &&rawInput);

        /** Destructor */
        ~CompleteParams() noexcept;

    };

    class Complete : public ParameterizedMexFunction<CompleteParams, MEXEntryPointID::Complete> {
    public:
        explicit Complete(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CompleteParams &input) override;

    };

}