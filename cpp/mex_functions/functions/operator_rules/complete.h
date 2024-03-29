/**
 * complete.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"
#include "integer_types.h"

#include "import/read_monomial_rules.h"

#include <memory>

namespace Moment::Algebraic {
    class NameTable;
    class AlgebraicPrecontext;
}

namespace Moment::mex::functions {

    class CompleteParams : public SortedInputs {
    public:
        size_t max_operators = 0;

        size_t max_attempts = 128;

        /** Precontext, including number of operators, and whether they are self-adjoint */
        std::unique_ptr<Algebraic::AlgebraicPrecontext> apc;

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

    class Complete : public ParameterizedMTKFunction<CompleteParams, MTKEntryPointID::Complete> {
    public:
        explicit Complete(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CompleteParams &input) override;

    };

}