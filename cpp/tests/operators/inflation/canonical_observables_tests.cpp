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

    TEST(Operators_Inflation_CanonicalObservables, Singleton) {
        InflationMatrixSystem ims{
                std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 1)};
        const auto& ic = ims.InflationContext();
        const auto& co = ims.CanonicalObservables();
        auto [mm, id] = ims.create_moment_matrix(1);
        ASSERT_EQ(co.size(), 2); // e, A.
    }

    TEST(Operators_Inflation_CanonicalObservables, Singleton_Cloned) {
        InflationMatrixSystem ims{
                std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2)};
        const auto& ic = ims.InflationContext();
        const auto& co = ims.CanonicalObservables();
        auto [mm, id] = ims.create_moment_matrix(1);
        EXPECT_EQ(ic.observable_variant_count(), 2); // a0, a1)
        ASSERT_EQ(co.size(), 3); // e, A0, A0A1
    }


    TEST(Operators_Inflation_CanonicalObservables, CVPair) {
        InflationMatrixSystem ims{
                std::make_unique<InflationContext>(CausalNetwork{{0, 0}, {{0, 1}}}, 1)};
        const auto& ic = ims.InflationContext();
        const auto& co = ims.CanonicalObservables();
        auto [mm, id] = ims.create_moment_matrix(1);
        ASSERT_EQ(co.size(), 6); // e, A, B, AA, AB, BB)

        const auto& canon_e = co.canonical(std::vector<OVIndex>{});
        EXPECT_EQ(canon_e.outcomes, 1) << co;
        EXPECT_EQ(canon_e.operators, 1);
        EXPECT_TRUE(canon_e.projective);
        ASSERT_EQ(canon_e.indices.size(), 0);

        const auto& canon_A = co.canonical(std::vector<OVIndex>{{0, 0}});
        EXPECT_EQ(canon_A.outcomes, 0);
        EXPECT_EQ(canon_A.operators, 1);
        EXPECT_FALSE(canon_A.projective);
        ASSERT_EQ(canon_A.indices.size(), 1);
        EXPECT_EQ(canon_A.indices[0].observable, 0);
        EXPECT_EQ(canon_A.indices[0].variant, 0);

        const auto& canon_B = co.canonical(std::vector<OVIndex>{{1, 0}});
        EXPECT_EQ(canon_B.outcomes, 0);
        EXPECT_EQ(canon_B.operators, 1);
        EXPECT_FALSE(canon_B.projective);
        ASSERT_EQ(canon_B.indices.size(), 1);
        EXPECT_EQ(canon_B.indices[0].observable, 1);
        EXPECT_EQ(canon_B.indices[0].variant, 0);

        const auto& canon_AA = co.canonical(std::vector<OVIndex>{{0, 0}, {0, 0}});
        EXPECT_EQ(canon_AA.outcomes, 0);
        EXPECT_EQ(canon_AA.operators, 1);
        EXPECT_FALSE(canon_AA.projective);
        ASSERT_EQ(canon_AA.indices.size(), 2);
        EXPECT_EQ(canon_AA.indices[0].observable, 0);
        EXPECT_EQ(canon_AA.indices[0].variant, 0);
        EXPECT_EQ(canon_AA.indices[1].observable, 0);
        EXPECT_EQ(canon_AA.indices[1].variant, 0);

        const auto& canon_AB = co.canonical(std::vector<OVIndex>{{0, 0}, {1, 0}});
        EXPECT_EQ(canon_AB.outcomes, 0);
        EXPECT_EQ(canon_AB.operators, 1);
        EXPECT_FALSE(canon_AB.projective);
        ASSERT_EQ(canon_AB.indices.size(), 2);
        EXPECT_EQ(canon_AB.indices[0].observable, 0);
        EXPECT_EQ(canon_AB.indices[0].variant, 0);
        EXPECT_EQ(canon_AB.indices[1].observable, 1);
        EXPECT_EQ(canon_AB.indices[1].variant, 0);

        const auto& canon_BB = co.canonical(std::vector<OVIndex>{{1, 0}, {1, 0}});
        EXPECT_EQ(canon_BB.outcomes, 0);
        EXPECT_EQ(canon_BB.operators, 1);
        EXPECT_FALSE(canon_BB.projective);
        ASSERT_EQ(canon_BB.indices.size(), 2);
        EXPECT_EQ(canon_BB.indices[0].observable, 1);
        EXPECT_EQ(canon_BB.indices[0].variant, 0);
        EXPECT_EQ(canon_BB.indices[1].observable, 1);
        EXPECT_EQ(canon_BB.indices[1].variant, 0);

        // Verify all indices are unique (and thus account for all canonical entries)
        const std::set all_indices = {canon_e.index, canon_A.index, canon_B.index,
                                      canon_AA.index, canon_AB.index, canon_BB.index};
        EXPECT_EQ(all_indices.size(), 6);
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