/**
 * new_algebraic_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "integer_types.h"
#include "import/read_monomial_rules.h"

namespace Moment::mex::functions {

    class NewAlgebraicMatrixSystemParams : public SortedInputs {
        public:
            size_t total_operators = 0;
            size_t complete_attempts = 0;
            bool hermitian_operators = true;
            bool commutative = false;

            std::vector<RawMonomialRule> rules{};

            NewAlgebraicMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine,
                                           SortedInputs &&rawInput);

    private:
        void getFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                       matlab::data::Array& input, const std::string& paramName);

    };

    class NewAlgebraicMatrixSystem : public Moment::mex::functions::MexFunction {
    public:
        explicit NewAlgebraicMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}