/**
 * generate_basis.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"

#include "symbolic/index_matrix_properties.h"


namespace NPATK::mex::functions {

    struct GenerateBasisParams : public SortedInputs {
    public:
        /** How is the input supplied to the basis generator */
        enum class InputMode {
            Unknown = 0,
            MATLABArray,
            MomentMatrixReference
        } input_mode = InputMode::Unknown;

        /** What sort of basis should we try to generate */
        IndexMatrixProperties::MatrixType basis_type = IndexMatrixProperties::MatrixType::Unknown;

        /** True, if output should be a sparse matrix */
        bool sparse_output = false;

    public:
        explicit GenerateBasisParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& structuredInputs);
    };

    class GenerateBasis : public MexFunction {
    public:
        explicit GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

    };

}