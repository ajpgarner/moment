/**
 * canonical_observables_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"
#include "operators/inflation/canonical_observables.h"

namespace NPATK::Tests {
    TEST(Operators_Inflation_CanonicalObservables, AliasTriangle) {
        InflationMatrixSystem ims{
            std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}}, 2)};
        const auto& ic = ims.InflationContext();
        const auto& co = ims.CanonicalObservables();
        auto [mm, id] = ims.create_moment_matrix(1);

        EXPECT_EQ(co.distinct_observables(0), 1);
        EXPECT_EQ(co.distinct_observables(1), 3);
        EXPECT_EQ(co.distinct_observables(2), 15);

        auto h_a00 = co.canonical(std::vector{OVIndex{0,0}});
        auto h_a01 = co.canonical(std::vector{OVIndex{0,1}});
        auto h_a10 = co.canonical(std::vector{OVIndex{0,2}});
        auto h_a11 = co.canonical(std::vector{OVIndex{0,3}});
        std::set A{h_a00, h_a01, h_a10, h_a11};
        EXPECT_EQ(A.size(), 1); // All map to A00


        auto h_a00a01 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,1}});
        auto h_a00a10 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,2}});
        auto h_a00a11 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,3}});
        std::set AA{h_a00a01, h_a00a10, h_a00a11};
        EXPECT_EQ(AA.size(), 3); // All map to A00
    }
}