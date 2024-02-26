/**
 * echo.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "echo_matrix.h"

#include "eigen/export_eigen_dense.h"
#include "eigen/export_eigen_sparse.h"
#include "eigen/read_eigen_dense.h"
#include "eigen/read_eigen_sparse.h"

#include "utilities/reporting.h"

namespace Moment::mex::functions {
    EchoMatrixParams::EchoMatrixParams(SortedInputs&& raw_input) : SortedInputs(std::move(raw_input)) {
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
                this->matrix_mode = MatrixMode::Real;
                break;
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                this->output_mode = OutputMode::Sparse;
                this->matrix_mode = MatrixMode::Real;
                break;
            case matlab::data::ArrayType::COMPLEX_DOUBLE:
                this->output_mode = OutputMode::Dense;
                this->matrix_mode = MatrixMode::Complex;
                break;
            case matlab::data::ArrayType::SPARSE_COMPLEX_DOUBLE:
                this->output_mode = OutputMode::Sparse;
                this->matrix_mode = MatrixMode::Complex;
                break;
            default:
                throw BadParameter{"Input type not supported"};
        }

        // Allow for override of default output type
        if (this->flags.contains(u"dense")) {
            this->output_mode = OutputMode::Dense;
        } else if (this->flags.contains(u"sparse")) {
            this->output_mode = OutputMode::Sparse;
        }

        // Allow for override of matrix output mode
        if (this->flags.contains(u"complex")) {
            this->matrix_mode = MatrixMode::Complex;
        } else if (this->flags.contains(u"real")) {
            this->matrix_mode = MatrixMode::Real;
        }
    }

    EchoMatrix::EchoMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 1;
        this->max_inputs = 1;
        this->min_outputs = 0;
        this->max_outputs = 1;

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"sparse", u"dense");

        this->flag_names.emplace(u"complex");
        this->flag_names.emplace(u"real");
        this->mutex_params.add_mutex(u"complex", u"real");

    }

    void EchoMatrix::operator()(IOArgumentRange output, EchoMatrixParams &input) {
        const bool output_to_console = this->verbose || (output.size() == 0);
        const bool output_to_matlab = output.size() > 0;

        if (input.output_mode == EchoMatrixParams::OutputMode::Dense) {

            if (input.matrix_mode == EchoMatrixParams::MatrixMode::Real) {
                auto eigen_dense_object = read_eigen_dense(this->matlabEngine, input.inputs[0]);
                if (output_to_console) {
                    std::stringstream ss;
                    ss << eigen_dense_object << "\n";
                    print_to_console(this->matlabEngine, ss.str());
                }

                if (output_to_matlab) {
                    matlab::data::ArrayFactory factory;
                    output[0] = export_eigen_dense(this->matlabEngine, factory, eigen_dense_object);
                }
                return;
            } else if (input.matrix_mode == EchoMatrixParams::MatrixMode::Complex) {
                auto eigen_dc_object = read_eigen_dense_complex(this->matlabEngine, input.inputs[0]);
                if (output_to_console) {
                    std::stringstream ss;
                    ss << eigen_dc_object << "\n";
                    print_to_console(this->matlabEngine, ss.str());
                }

                if (output_to_matlab) {
                    matlab::data::ArrayFactory factory;
                    output[0] = export_eigen_dense(this->matlabEngine, factory, eigen_dc_object);
                }
                return;
            }
            throw InternalError{"Unsupported/unknown MatrixMode."};
        }

        if (input.output_mode == EchoMatrixParams::OutputMode::Sparse) {

            if (input.matrix_mode == EchoMatrixParams::MatrixMode::Real) {
                auto eigen_sparse_object = read_eigen_sparse(this->matlabEngine, input.inputs[0]);
                if (output_to_console) {
                    std::stringstream ss;
                    ss << eigen_sparse_object << "\n";
                    print_to_console(this->matlabEngine, ss.str());
                }

                if (output_to_matlab) {
                    matlab::data::ArrayFactory factory;
                    output[0] = export_eigen_sparse(this->matlabEngine, factory, eigen_sparse_object);
                }
                return;
            } else if (input.matrix_mode == EchoMatrixParams::MatrixMode::Complex) {
                auto eigen_sc_object = read_eigen_sparse_complex(this->matlabEngine, input.inputs[0]);
                if (output_to_console) {
                    std::stringstream ss;
                    ss << eigen_sc_object << "\n";
                    print_to_console(this->matlabEngine, ss.str());
                }

                if (output_to_matlab) {
                    matlab::data::ArrayFactory factory;
                    output[0] = export_eigen_sparse(this->matlabEngine, factory, eigen_sc_object);
                }
                return;
            }
            throw InternalError{"Unsupported/unknown MatrixMode."};
        }
        throw InternalError{"Unsupported/unknown OutputMode."};
    }
}