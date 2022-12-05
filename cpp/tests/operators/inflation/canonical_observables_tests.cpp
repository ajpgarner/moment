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
      TEST(Operators_Inflation_CanonicalObservables, Hash) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {{0}, {0, 1}}}, 2)};
        const auto& ic = ims.InflationContext();
        const auto& co = ims.CanonicalObservables();
        
        size_t hash_e = co.hash(std::vector<OVIndex>{});
        size_t hash_a0 = co.hash(std::vector{OVIndex(0LL, 0LL)});
        size_t hash_a1 = co.hash(std::vector{OVIndex(0LL, 1LL)});
        size_t hash_a2 = co.hash(std::vector{OVIndex(0LL, 2LL)});
        size_t hash_a3 = co.hash(std::vector{OVIndex(0LL, 3LL)});
        size_t hash_b0 = co.hash(std::vector{OVIndex(1LL, 0LL)});
        size_t hash_b1 = co.hash(std::vector{OVIndex(1LL, 1LL)});

        EXPECT_LT(hash_e, hash_a0);
        EXPECT_LT(hash_a0, hash_a1);
        EXPECT_LT(hash_a1, hash_a2);
        EXPECT_LT(hash_a2, hash_a3);
        EXPECT_LT(hash_a3, hash_b0);
        EXPECT_LT(hash_b0, hash_b1);

        size_t last_hash = hash_b1;
        std::set<size_t> a_hashes;
        for (oper_name_t a_var = 0; a_var < 4; ++a_var) {
            size_t hash = co.hash(std::vector{OVIndex(0LL, 0LL), OVIndex(0LL, a_var)});
            a_hashes.emplace(hash);
            EXPECT_LT(hash_b1, hash);
            EXPECT_LT(last_hash, hash);
            last_hash = hash;
        }
        EXPECT_EQ(a_hashes.size(), 4);

        for (oper_name_t b_var = 0; b_var < 2; ++b_var) {
            size_t hash = co.hash(std::vector{OVIndex(0LL, 0LL), OVIndex(1LL, b_var)});
            a_hashes.emplace(hash);
            EXPECT_LT(hash_b1, hash);
            EXPECT_LT(last_hash, hash);
            last_hash = hash;
        }
        EXPECT_EQ(a_hashes.size(), 6);
    }

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
        std::set A{h_a00.hash, h_a01.hash, h_a10.hash, h_a11.hash};
        EXPECT_EQ(A.size(), 1); // All map to A00

        auto h_a00a01 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,1}});
        auto h_a00a10 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,2}});
        auto h_a00a11 = co.canonical(std::vector{OVIndex{0,0}, OVIndex{0,3}});
        std::set AA{h_a00a01.hash, h_a00a10.hash, h_a00a11.hash};
        EXPECT_EQ(AA.size(), 3); // All are distinct
    }

}