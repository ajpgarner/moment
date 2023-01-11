/**
 * operator_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "integer_types.h"

#include <string>

namespace Moment {
    class MatrixSystem;
    class SymbolicMatrix;
}

namespace Moment::mex::functions  {

    struct OperatorMatrixParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

        enum class OutputMode {
            /** Unknown output */
            Unknown = 0,
            /** Output index and dimension of matrix */
            IndexAndDimension,
            /** Output matrix of symbol names */
            Symbols,
            /** Output matrix of string representation of operator sequences */
            Sequences
        } output_mode = OutputMode::Unknown;

    public:
        OperatorMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs)
            : SortedInputs(std::move(inputs)) { }

        void parse(matlab::engine::MATLABEngine &matlabEngine);

    protected:
        virtual void extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) = 0;

        virtual void extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) = 0;

        /** True if reference id, or derived parameter (e.g. level, word, etc.), set */
        [[nodiscard]] virtual bool any_param_set() const;

        /** Number of inputs required to fully specify matrix requested */
        [[nodiscard]] virtual size_t inputs_required() const noexcept { return 1; }

        /** Correct format */
        [[nodiscard]] virtual std::string input_format() const { return "[matrix system ID]"; }
    };

    struct RawOperatorMatrixParams : public OperatorMatrixParams {
    public:
        uint64_t matrix_index = 0;

    public:
        RawOperatorMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs)
            : OperatorMatrixParams(matlabEngine, std::move(inputs)) { }

    protected:
        void extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) final;

        void extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) final;

        /** True if reference id, or derived parameter (e.g. level, word, etc.), set */
        [[nodiscard]] bool any_param_set() const final;

        /** Number of inputs required to fully specify matrix requested */
        [[nodiscard]] size_t inputs_required() const noexcept final { return 2; }

        /** Correct format */
        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, matrix index]"; }
    };


    class OperatorMatrix : public Moment::mex::functions::MexFunction {
    protected:
        OperatorMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage,
                       MEXEntryPointID id, std::basic_string<char16_t> name);

    public:
        OperatorMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : OperatorMatrix(matlabEngine, storage, MEXEntryPointID::OperatorMatrix, u"operator_matrix") { }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) override;

        [[nodiscard]] std::unique_ptr<SortedInputs>
        transform_inputs(std::unique_ptr<SortedInputs> input) const override;

    protected:
        /**
         * Query matrix system for requested matrix.
         * @return Pair: Index of matrix, reference to matrix.
         */
        virtual std::pair<size_t, const Moment::SymbolicMatrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp);

    };
}
