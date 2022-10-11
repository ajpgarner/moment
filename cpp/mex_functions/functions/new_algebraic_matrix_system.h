/**
 * new_algebraic_matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "integer_types.h"

namespace NPATK::mex::functions {

    class NewAlgebraicMatrixSystemParams : public SortedInputs {
    public:
        struct RawMonomialRule {
            std::vector<oper_name_t> LHS{};
            std::vector<oper_name_t> RHS{};

            RawMonomialRule() = default;

            RawMonomialRule(std::vector<oper_name_t>&& lhs, std::vector<oper_name_t>&& rhs)
                : LHS(std::move(lhs)), RHS(std::move(rhs)) { }

        };

        public:
            size_t total_operators = 0;
            std::vector<RawMonomialRule> rules{};

            NewAlgebraicMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine,
                                           SortedInputs &&rawInput);

    private:
        void getFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                       matlab::data::Array& input, const std::string& paramName);

        void readRulesSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                       matlab::data::Array& input, const std::string& paramName);

    };

    class NewAlgebraicMatrixSystem : public NPATK::mex::functions::MexFunction {
    public:
        explicit NewAlgebraicMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}