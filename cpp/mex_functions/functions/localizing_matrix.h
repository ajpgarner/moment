/**
 * localizing_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_matrix.h"
#include "matrix/localizing_matrix_index.h"

namespace Moment {
    class Context;
}

namespace Moment::mex::functions  {

    struct LocalizingMatrixParams : public OperatorMatrixParams {
    public:
        unsigned long hierarchy_level = 0;

        bool matlab_indexing = false;

        std::vector<oper_name_t> localizing_word;

    public:
        explicit LocalizingMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs)
            : OperatorMatrixParams(matlabEngine, std::move(inputs)) { }

        /**
         * Use the supplied context to create an index for the requested localizing matrix.
         */
        LocalizingMatrixIndex to_index(matlab::engine::MATLABEngine &matlabEngine, const Context& context) const;



    protected:
        void extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) final;

        void extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) final;

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level, word]"; }
    };

    class LocalizingMatrix : public Moment::mex::functions::OperatorMatrix {
    public:
        explicit LocalizingMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, const OperatorMatrixParams& omp) final;
    };
}
