/**
 * extended_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_matrix.h"
#include "integer_types.h"

namespace Moment::mex::functions {

    class ExtendedMatrixParams : public OperatorMatrixParams {
    public:
        size_t hierarchy_level = 0;
        std::vector<symbol_name_t> extensions;

        enum class ExtensionType {
            Manual,
            Automatic
        } extension_type = ExtensionType::Manual;

        explicit ExtendedMatrixParams(SortedInputs&& input) : OperatorMatrixParams(std::move(input)) { }

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 3; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level, extensions]"; }

    private:
        void read_extension_argument(const std::string& paramName,
                                     const matlab::data::Array& input_array);
    };

    class ExtendedMatrix
        : public Moment::mex::functions::OperatorMatrix<ExtendedMatrixParams, MEXEntryPointID::ExtendedMatrix> {
    public:
        explicit ExtendedMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        std::pair<size_t, const Moment::MonomialMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) override;

    };
}