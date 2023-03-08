/**
 * echo.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "echo.h"

#include "eigen/export_eigen_dense.h"
#include "eigen/export_eigen_sparse.h"
#include "eigen/read_eigen_dense.h"
#include "eigen/read_eigen_sparse.h"

#include "utilities/reporting.h"

namespace Moment::mex::functions {
    EchoParams::EchoParams(SortedInputs&& raw_input) : SortedInputs(std::move(raw_input)) {
        switch (inputs[0].getType()) {
            case matlab::data::ArrayType::MATLAB_STRING:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
                this->output_mode = OutputMode::Dense;
                break;
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                this->output_mode = OutputMode::Sparse;
                break;
            default:
                throw_error(this->matlabEngine, errors::bad_param, u"Input type not supported");
        }

        // Allow for override of default output type
        if (this->flags.contains(u"dense")) {
            this->output_mode = OutputMode::Dense;
        } else if (this->flags.contains(u"sparse")) {
            this->output_mode = OutputMode::Sparse;
        }
    }

    Echo::Echo(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMexFunction<EchoParams, MEXEntryPointID::Echo>(matlabEngine, storage, u"echo") {
        this->min_inputs = 1;
        this->max_inputs = 1;
        this->min_outputs = 0;
        this->max_outputs = 1;

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"sparse", u"dense");

    }
    void Echo::operator()(IOArgumentRange output, EchoParams &input) {
        const bool output_to_console = this->verbose || (output.size() == 0);
        const bool output_to_matlab = output.size() > 0;

        if (input.output_mode == EchoParams::OutputMode::Dense) {

            auto eigen_dense_object = read_eigen_dense(this->matlabEngine, input.inputs[0]);
            if (output_to_console) {
                std::stringstream ss;
                ss << eigen_dense_object << "\n";
                print_to_console(this->matlabEngine, ss.str());
            }

            if (output_to_matlab) {
                output[0] = export_eigen_dense(this->matlabEngine, eigen_dense_object);
            }
            return;
        }

        if (input.output_mode == EchoParams::OutputMode::Sparse) {

            auto eigen_sparse_object = read_eigen_sparse(this->matlabEngine, input.inputs[0]);
            if (output_to_console) {
                std::stringstream ss;
                ss << eigen_sparse_object << "\n";
                print_to_console(this->matlabEngine, ss.str());
            }

            if (output_to_matlab) {
                output[0] = export_eigen_sparse(this->matlabEngine, eigen_sparse_object);
            }
            return;
        }
        throw_error(this->matlabEngine, errors::internal_error, "Unsupported/unknown OutputMode.");
    }
}