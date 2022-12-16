/**
 * factor_table_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/matrix/symbol_table.h"
#include "operators/inflation/canonical_observables.h"
#include "operators/inflation/factor_table.h"
#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

namespace NPATK::Tests {
    TEST(Operators_Inflation_FactorTable, Empty) {
        std::unique_ptr<InflationContext> icPtr
            = std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {{0, 1}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& factors = ims.Factors();

        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 2);

        // Zero
        const auto& factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);
    }


    TEST(Operators_Inflation_FactorTable, UnlinkedPair) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& factors = ims.Factors();

        const auto& mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, AB
        ASSERT_EQ(ims.Symbols().size(), 5);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 5);

        // Zero
        const auto& factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);
        
        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);

        // AB -> A, B
        const auto& factors_AB = factors[4];
        EXPECT_EQ(factors_AB.id, 4);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.raw.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.canonical.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AB.canonical.symbols[1], 3);
    }

    TEST(Operators_Inflation_FactorTable, UnlinkedCVPair) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& factors = ims.Factors();
        const auto& symbols = ims.Symbols();

        const auto& mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, A^2, AB, B^2
        ASSERT_EQ(symbols.size(), 7);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 7);

        // Zero
        const auto& factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);

        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);

        // AA
        const auto& factors_AA = factors[4];
        EXPECT_EQ(factors_AA.id, 4);
        ASSERT_EQ(factors_AA.raw.sequences.size(), 1) << symbols << ims.CanonicalObservables();
        EXPECT_EQ(factors_AA.raw.sequences[0], OperatorSequence({0, 0}, context));
        ASSERT_EQ(factors_AA.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_AA.canonical.sequences[0], OperatorSequence({0, 0}, context));
        ASSERT_EQ(factors_AA.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_AA.canonical.symbols[0], 4);
        
        // AB -> A, B
        const auto& factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.raw.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.canonical.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AB.canonical.symbols[1], 3);

        // BB
        const auto& factors_BB = factors[6];
        EXPECT_EQ(factors_BB.id, 6);
        ASSERT_EQ(factors_BB.raw.sequences.size(), 1);
        EXPECT_EQ(factors_BB.raw.sequences[0], OperatorSequence({1, 1}, context));
        ASSERT_EQ(factors_BB.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_BB.canonical.sequences[0], OperatorSequence({1, 1}, context));
        ASSERT_EQ(factors_BB.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_BB.canonical.symbols[0], 6);
    }

    TEST(Operators_Inflation_FactorTable, W) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& factors = ims.Factors();

        const auto& mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, C, AB, AC, BC
        ASSERT_EQ(ims.Symbols().size(), 8);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 8);

        // Zero
        const auto& factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);

        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);

        // C
        const auto& factors_C = factors[4];
        EXPECT_EQ(factors_C.id, 4);
        ASSERT_EQ(factors_C.raw.sequences.size(), 1);
        EXPECT_EQ(factors_C.raw.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_C.canonical.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_C.canonical.symbols[0], 4);

        // AB -> AB
        const auto& factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 1);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 5);
        
        // AC -> A, C
        const auto& factors_AC = factors[6];
        EXPECT_EQ(factors_AC.id, 6);
        ASSERT_EQ(factors_AC.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AC.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.raw.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AC.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.canonical.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AC.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AC.canonical.symbols[1], 4);

        // BC -> BC
        const auto& factors_BC = factors[7];
        EXPECT_EQ(factors_BC.id, 7);
        ASSERT_EQ(factors_BC.raw.sequences.size(), 1);
        EXPECT_EQ(factors_BC.raw.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_BC.canonical.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_BC.canonical.symbols[0], 7);
    }
}