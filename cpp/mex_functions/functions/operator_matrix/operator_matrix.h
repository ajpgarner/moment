/**
 * operator_matrix.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"
#include "integer_types.h"

#include <concepts>
#include <string>

namespace Moment {
    class Matrix;
    class MatrixSystem;
    class MonomialMatrix;
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
            Sequences,
            /** Output basis indices and masks associated with matrix */
            Masks
        } output_mode = OutputMode::Unknown;

    public:
        explicit OperatorMatrixParams(SortedInputs&& inputs) : SortedInputs(std::move(inputs)) { }

        void parse();

    protected:
        virtual void extra_parse_params() = 0;

        virtual void extra_parse_inputs() = 0;

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
        explicit RawOperatorMatrixParams(SortedInputs&& inputs) : OperatorMatrixParams(std::move(inputs)) { }

    protected:
        void extra_parse_params() final;

        void extra_parse_inputs() final;

        /** True if reference id, or derived parameter (e.g. level, word, etc.), set */
        [[nodiscard]] bool any_param_set() const final;

        /** Number of inputs required to fully specify matrix requested */
        [[nodiscard]] size_t inputs_required() const noexcept final { return 2; }

        /** Correct format */
        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, matrix index]"; }
    };

    class OperatorMatrixVirtualBase {
    private:
        matlab::engine::MATLABEngine& omvb_matlabEngine;
        StorageManager& omvb_storageManager;

    protected:
        OperatorMatrixVirtualBase(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : omvb_matlabEngine{matlabEngine}, omvb_storageManager{storage} { }

    public:
        virtual ~OperatorMatrixVirtualBase() = default;

    protected:
        void process(IOArgumentRange output, OperatorMatrixParams& input);

        void check_mat_sys_id(OperatorMatrixParams &input) const;

        void do_validate_output_count(size_t outputs, const OperatorMatrixParams& inputs) const;

        /**
         * Query matrix system for requested matrix.
         * @return Pair: Index of matrix, reference to matrix.
         */
        virtual std::pair<size_t, const Moment::Matrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) = 0;

        /**
         * Get settings object
         */
        virtual const EnvironmentalVariables& omvb_settings() const = 0;

    };

    template<std::derived_from<OperatorMatrixParams> om_param_t, MEXEntryPointID om_entry_id>
    class OperatorMatrix : public ParameterizedMexFunction<om_param_t, om_entry_id>,
                           public OperatorMatrixVirtualBase {
    protected:
        OperatorMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
           : ParameterizedMexFunction<om_param_t, om_entry_id>{matlabEngine, storage},
             OperatorMatrixVirtualBase{matlabEngine, storage}
        {
            this->min_outputs = 1;
            this->max_outputs = 4;

            this->flag_names.emplace(u"sequences");
            this->flag_names.emplace(u"symbols");
            this->flag_names.emplace(u"dimension");
            this->flag_names.emplace(u"masks");

            this->param_names.emplace(u"reference_id");
            this->param_names.emplace(u"index");

            // One of four ways to output:
            this->mutex_params.add_mutex({u"sequences", u"symbols", u"dimension", u"masks"});

            // Either [ref, ref] or named version thereof.
            this->min_inputs = 0;
            this->max_inputs = 2;
        }

        void operator()(IOArgumentRange output, om_param_t &input) final {
            this->process(output, input);
        }

        void extra_input_checks(om_param_t &input) const final {
            input.parse();
            this->check_mat_sys_id(input);
        }

        void validate_output_count(size_t outputs, const SortedInputs &inputs) const override {
            const auto& cast_inputs = dynamic_cast<const OperatorMatrixParams&>(inputs);
            this->do_validate_output_count(outputs, cast_inputs);
        }

        const EnvironmentalVariables& omvb_settings() const final {
            return *this->settings;
        }
    };

    class RawOperatorMatrix : public OperatorMatrix<RawOperatorMatrixParams, MEXEntryPointID::OperatorMatrix> {
    public:
        RawOperatorMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : OperatorMatrix{matlabEngine, storage} { }

    protected:
        std::pair<size_t, const Moment::Matrix&>
        get_or_make_matrix(MatrixSystem& system, OperatorMatrixParams &omp) final;
    };
}
