/**
 * lattice_symmetrize.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mtk_function.h"
#include "integer_types.h"

#include "import/matrix_system_id.h"
#include "import/read_opseq_polynomial.h"

#include <string>

namespace Moment::mex::functions {

    struct LatticeSymmetrizeParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        std::unique_ptr<StagingPolynomial> input_polynomial;

    public:
        explicit LatticeSymmetrizeParams(SortedInputs&& inputs);

    };

    class LatticeSymmetrize : public ParameterizedMTKFunction<LatticeSymmetrizeParams, MTKEntryPointID::LatticeSymmetrize> {
    public:
        LatticeSymmetrize(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, LatticeSymmetrizeParams& input) override;
    };
}