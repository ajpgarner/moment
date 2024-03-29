/**
 * moment_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions  {

    struct MomentMatrixParams : public OperatorMatrixParams {
    public:
        size_t hierarchy_level = 0;
        struct extra_data_t {
            size_t nearest_neighbours = 0;
        } extra_data;
    public:

        explicit MomentMatrixParams(SortedInputs&& inputs) : OperatorMatrixParams(std::move(inputs)) { }

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        void parse_optional_params();

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 2; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level]"; }
    };

    class MomentMatrix
        : public Moment::mex::functions::OperatorMatrix<MomentMatrixParams, MTKEntryPointID::MomentMatrix> {
    public:
        MomentMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}
