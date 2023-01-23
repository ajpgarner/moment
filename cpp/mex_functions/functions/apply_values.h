/**
 * apply_values.h
 *
 * Copyright (c) 2023 Austrian Academy of Sciences
 */

#include "mex_function.h"
#include "operator_matrix.h"

#include "integer_types.h"

#include <string>

namespace Moment::mex::functions  {

    struct ApplyValuesParams : public OperatorMatrixParams {
    public:
        uint64_t matrix_index = 0;
        std::map<symbol_name_t, double> substitutions;

    public:
        ApplyValuesParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs)
        : OperatorMatrixParams(matlabEngine, std::move(inputs)) { }

    protected:
        void extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) final;

        void extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) final;

        /** True if reference id, or derived parameter (e.g. level, word, etc.), set */
        [[nodiscard]] bool any_param_set() const final;

        /** Number of inputs required to fully specify matrix requested */
        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        /** Correct format */
        [[nodiscard]] std::string input_format() const final {
            return "[matrix system ID, matrix index, substitution list]";
        }

        static std::map<symbol_name_t, double>
        read_substitution_cell(matlab::engine::MATLABEngine &engine, const std::string &param_str,
                               const matlab::data::Array &input);
    };

    class ApplyValues : public Moment::mex::functions::OperatorMatrix<ApplyValuesParams, MEXEntryPointID::ApplyValues> {
    public:
        ApplyValues(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}