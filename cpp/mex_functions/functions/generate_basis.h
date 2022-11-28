/**
 * generate_basis.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"

#include "operators/matrix/operator_matrix.h"
//#include "symbolic/index_matrix_properties.h"


namespace NPATK::mex::functions {

    struct GenerateBasisParams : public SortedInputs {
    public:
        /** How is the input supplied to the basis generator */
        enum class InputMode {
            Unknown = 0,
            MATLABArray,
            MatrixSystemReference
        } input_mode = InputMode::Unknown;

        /** What sort of basis should we try to generate */
        MatrixType basis_type = MatrixType::Unknown;

        /** True, if output should be a sparse matrix */
        bool sparse_output = false;

        /** True, if output should be an indexed sparse array, or a flattened monolithic array */
        bool monolithic_output = false;

        /** The reference to the moment matrix, if one is requested */
        uint64_t matrix_system_key = 0;

        /** The reference to the matrix within the system, if one is requested */
        uint64_t matrix_index = 0;

    public:
        explicit GenerateBasisParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);

        [[nodiscard]] bool complex_output() const noexcept {
            return (this->basis_type == MatrixType::Hermitian) || (this->basis_type == MatrixType::Complex);
        }
    };

    class GenerateBasis : public MexFunction {
    public:
        explicit GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

    private:
        void doGenerateBasisArray(std::array<matlab::data::Array, 3>& output, GenerateBasisParams& params);

        void doGenerateBasisOperatorMatrix(std::array<matlab::data::Array, 3>& output, GenerateBasisParams& params);

    };

}