/**
 * pauli_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../../mtk_function.h"


namespace Moment::mex::functions {
    struct PauliMatrixSystemParams : public SortedInputs {
    public:
        /** The number of qubit sites. */
        size_t qubit_count = 0;

        /** The row size, if a 2D lattice; 0 for a chain */
        size_t row_width = 0;

        /** True to wrap or tile the qubits for the purpose of identifying neighbours */
        bool wrap = false;

        /** True to automatically symmetrize */
        bool symmetrized = false;

        /** The 'precision' limit, such that if a value is less than eps * [this value] it is treated as zero. */
        double zero_tolerance = 1.0;

    public:
        explicit PauliMatrixSystemParams(SortedInputs &&inputs);
    };


    class PauliMatrixSystem : public ParameterizedMTKFunction<PauliMatrixSystemParams,
            MTKEntryPointID::PauliMatrixSystem> {
    public:
        explicit PauliMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, PauliMatrixSystemParams& input) override;

    };
}