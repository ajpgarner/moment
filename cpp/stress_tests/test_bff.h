/**
 * test_bff.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/algebraic/algebraic_matrix_system.h"

#include <memory>

namespace Moment::StressTests {
    class BrownFawziFawzi {
    public:
        const size_t M;

    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;

    public:
        BrownFawziFawzi(size_t M);

        void set_up_ams();

        const Moment::SymbolicMatrix& make_moment_matrix(size_t mm_level);
    };
}