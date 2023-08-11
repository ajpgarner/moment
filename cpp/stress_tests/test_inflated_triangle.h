/**
 * test_inflated_triangle.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "scenarios/inflation/inflation_matrix_system.h"

#include <memory>


namespace Moment::StressTests {
    class InflatedTriangle {
    public:
        const size_t OutcomesPerCorner;
        const size_t InflationLevel;

    private:
        std::unique_ptr<Inflation::InflationMatrixSystem> ims_ptr;

    public:
        InflatedTriangle(size_t outcomes, size_t inflation_level);

        void set_up_ims();

        const Moment::SymbolicMatrix& make_moment_matrix(size_t mm_level);

        const Moment::SymbolicMatrix& make_extended_matrix(size_t mm_level);
    };
}