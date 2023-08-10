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
        const size_t mm_level;

    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;

        const Moment::SymbolicMatrix * moment_matrix;

    public:
        BrownFawziFawzi(size_t M, size_t mm_level);

        void set_up_ams();

        void make_moment_matrix();
    };
}