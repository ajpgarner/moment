/**
 * inflation_full_correlator_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_full_correlator.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../probability_tensor_test_helpers.h"

#include <set>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_FullCorrelator, Empty) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        ims.RefreshCollinsGisin();

        InflationFullCorrelator fc{ims};

    }
}