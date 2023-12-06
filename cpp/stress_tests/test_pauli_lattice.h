/**
 * test_pauli_lattice.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/pauli/pauli_matrix_system.h"


#include <Eigen/Sparse>

#include <memory>
#include <vector>

namespace Moment::StressTests {
    class PauliLattice {
    public:
        const size_t column_height;
        const size_t row_width;

    private:
        std::shared_ptr<Pauli::PauliMatrixSystem> pms_ptr;

    public:
        PauliLattice(size_t col_height, size_t row_width);

        Pauli::PauliMatrixSystem& make_pms();

        [[nodiscard]] inline Pauli::PauliMatrixSystem& pms() const noexcept {
            return *pms_ptr;
        }

        bool test_moment_matrix(Pauli::NearestNeighbourIndex nni);
    };
}

