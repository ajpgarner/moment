/**
 * factor_table_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/matrix/symbol_table.h"
#include "operators/inflation/factor_table.h"
#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

namespace NPATK::Tests {
    TEST(FactorTable, Empty) {
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
        ASSERT_EQ(factors_0.sequences.size(), 1);
        EXPECT_EQ(factors_0.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.symbols.size(), 1);
        EXPECT_EQ(factors_0.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.sequences.size(), 1);
        EXPECT_EQ(factors_I.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.symbols.size(), 1);
        EXPECT_EQ(factors_I.symbols[0], 1);
    }


    TEST(FactorTable, UnlinkedPair) {
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
        ASSERT_EQ(factors_0.sequences.size(), 1);
        EXPECT_EQ(factors_0.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.symbols.size(), 1);
        EXPECT_EQ(factors_0.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.sequences.size(), 1);
        EXPECT_EQ(factors_I.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.symbols.size(), 1);
        EXPECT_EQ(factors_I.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.sequences.size(), 1);
        EXPECT_EQ(factors_A.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.symbols.size(), 1);
        EXPECT_EQ(factors_A.symbols[0], 2);
        
        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.sequences.size(), 1);
        EXPECT_EQ(factors_B.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.symbols.size(), 1);
        EXPECT_EQ(factors_B.symbols[0], 3);

        // AB -> A, B
        const auto& factors_AB = factors[4];
        EXPECT_EQ(factors_AB.id, 4);
        ASSERT_EQ(factors_AB.sequences.size(), 2);
        EXPECT_EQ(factors_AB.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.symbols.size(), 2);
        EXPECT_EQ(factors_AB.symbols[0], 2);
        EXPECT_EQ(factors_AB.symbols[1], 3);
    }

    TEST(FactorTable, W) {
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
        ASSERT_EQ(factors_0.sequences.size(), 1);
        EXPECT_EQ(factors_0.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.symbols.size(), 1);
        EXPECT_EQ(factors_0.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.sequences.size(), 1);
        EXPECT_EQ(factors_I.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.symbols.size(), 1);
        EXPECT_EQ(factors_I.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.sequences.size(), 1);
        EXPECT_EQ(factors_A.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.symbols.size(), 1);
        EXPECT_EQ(factors_A.symbols[0], 2);

        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.sequences.size(), 1);
        EXPECT_EQ(factors_B.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.symbols.size(), 1);
        EXPECT_EQ(factors_B.symbols[0], 3);

        // C
        const auto& factors_C = factors[4];
        EXPECT_EQ(factors_C.id, 4);
        ASSERT_EQ(factors_C.sequences.size(), 1);
        EXPECT_EQ(factors_C.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.symbols.size(), 1);
        EXPECT_EQ(factors_C.symbols[0], 4);

        // AB -> AB
        const auto& factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        ASSERT_EQ(factors_AB.sequences.size(), 1);
        EXPECT_EQ(factors_AB.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.symbols.size(), 1);
        EXPECT_EQ(factors_AB.symbols[0], 5);
        
        // AC -> A, C
        const auto& factors_AC = factors[6];
        EXPECT_EQ(factors_AC.id, 6);
        ASSERT_EQ(factors_AC.sequences.size(), 2);
        EXPECT_EQ(factors_AC.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.symbols.size(), 2);
        EXPECT_EQ(factors_AC.symbols[0], 2);
        EXPECT_EQ(factors_AC.symbols[1], 4);

        // BC -> BC
        const auto& factors_BC = factors[7];
        EXPECT_EQ(factors_BC.id, 7);
        ASSERT_EQ(factors_BC.sequences.size(), 1);
        EXPECT_EQ(factors_BC.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.symbols.size(), 1);
        EXPECT_EQ(factors_BC.symbols[0], 7);
    }

    TEST(FactorTable, FromLocalizingMatrix) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& factors = ims.Factors();
        const auto& symbols = ims.Symbols();

        auto A0 = context.Observables()[0].variant(std::vector<oper_name_t>{0}).operator_offset;
        auto A1 = context.Observables()[0].variant(std::vector<oper_name_t>{1}).operator_offset;
        auto B00 = context.Observables()[1].variant(std::vector<oper_name_t>{0,0}).operator_offset;
        auto B01 = context.Observables()[1].variant(std::vector<oper_name_t>{0,1}).operator_offset;
        auto B10 = context.Observables()[1].variant(std::vector<oper_name_t>{1,0}).operator_offset;
        auto B11 = context.Observables()[1].variant(std::vector<oper_name_t>{1,1}).operator_offset;
        auto C0 = context.Observables()[2].variant(std::vector<oper_name_t>{0}).operator_offset;
        auto C1 = context.Observables()[2].variant(std::vector<oper_name_t>{1}).operator_offset;

        const auto& lm = ims.create_localizing_matrix(LocalizingMatrixIndex{context, 0, OperatorSequence{{A0,A1}, context}});

        // Get A0A1 symbol
        auto A0A1_ptr = symbols.where(OperatorSequence{{A0, A1}, context});
        ASSERT_NE(A0A1_ptr, nullptr);
        auto id_A0A1 = A0A1_ptr->Id();

        // Get A0 symbol
        auto A0_ptr = symbols.where(OperatorSequence{{A0}, context});
        ASSERT_NE(A0_ptr, nullptr);
        auto id_A0 = A0_ptr->Id();

        // Get A1 symbol
        auto A1_ptr = symbols.where(OperatorSequence{{A1}, context});
        ASSERT_NE(A1_ptr, nullptr);
        auto id_A1 = A1_ptr->Id();

        // Verify factorization of A0A1
        ASSERT_GE(factors.size(), id_A0A1);
        const auto& factors_A0A1 = factors[id_A0A1];
        EXPECT_EQ(factors_A0A1.id, id_A0A1);
        ASSERT_EQ(factors_A0A1.sequences.size(), 2);
        EXPECT_EQ(factors_A0A1.sequences[0], OperatorSequence({A0}, context));
        EXPECT_EQ(factors_A0A1.sequences[1], OperatorSequence({A1}, context));
        ASSERT_EQ(factors_A0A1.symbols.size(), 2);
        EXPECT_EQ(factors_A0A1.symbols[0], id_A0);
        EXPECT_EQ(factors_A0A1.symbols[1], id_A1);

        // A0 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A0);
        const auto& factors_A0 = factors[id_A0];
        EXPECT_EQ(factors_A0.id, id_A0);
        ASSERT_EQ(factors_A0.sequences.size(), 1);
        EXPECT_EQ(factors_A0.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0.symbols.size(), 1);
        EXPECT_EQ(factors_A0.symbols[0], id_A0);

        // A1 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A1);
        const auto& factors_A1 = factors[id_A1];
        EXPECT_EQ(factors_A1.id, id_A1);
        ASSERT_EQ(factors_A1.sequences.size(), 1);
        EXPECT_EQ(factors_A1.sequences[0], OperatorSequence({A1}, context));
        ASSERT_EQ(factors_A1.symbols.size(), 1);
        EXPECT_EQ(factors_A1.symbols[0], id_A1);
    }
}