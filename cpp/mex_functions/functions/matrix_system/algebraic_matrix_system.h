/**
 * algebraic_matrix_system.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"
#include "integer_types.h"
#include "import/read_monomial_rules.h"

#include <memory>
#include <vector>

namespace Moment::Algebraic {
    class AlgebraicPrecontext;
    class NameTable;
}

namespace Moment::mex::functions {

    class AlgebraicMatrixSystemParams : public SortedInputs {
        public:
            size_t total_operators = 0;
            size_t complete_attempts = 0;
            bool normal_operators = true;
            bool commutative = false;

            std::vector<RawMonomialRule> rules{};

            std::unique_ptr<Algebraic::AlgebraicPrecontext> apc;

            std::unique_ptr<Algebraic::NameTable> names;

    public:
            explicit AlgebraicMatrixSystemParams(SortedInputs &&rawInput);

            ~AlgebraicMatrixSystemParams();

    private:
        void getFromParams(matlab::engine::MATLABEngine &matlabEngine);

        void getFromInputs(matlab::engine::MATLABEngine &matlabEngine);

        void readOperatorSpecification(matlab::engine::MATLABEngine &matlabEngine,
                                       matlab::data::Array& input, const std::string& paramName);

    };

    class AlgebraicMatrixSystem
        : public ParameterizedMexFunction<AlgebraicMatrixSystemParams, MEXEntryPointID::AlgebraicMatrixSystem> {
    public:
        explicit AlgebraicMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, AlgebraicMatrixSystemParams &input) override;

    };

}