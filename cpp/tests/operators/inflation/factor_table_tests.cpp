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
        EXPECT_EQ(factors_0.canonical_id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.raw.symbols.size(), 1);
        EXPECT_EQ(factors_0.raw.symbols[0], 0);
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        EXPECT_EQ(factors_I.canonical_id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.raw.symbols.size(), 1);
        EXPECT_EQ(factors_I.raw.symbols[0], 1);
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);
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
        EXPECT_EQ(factors_0.canonical_id, 0);
        EXPECT_TRUE(factors_0.is_canonical());
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.raw.symbols.size(), 1);
        EXPECT_EQ(factors_0.raw.symbols[0], 0);
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        EXPECT_EQ(factors_I.canonical_id, 1);
        EXPECT_TRUE(factors_I.is_canonical());
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.raw.symbols.size(), 1);
        EXPECT_EQ(factors_I.raw.symbols[0], 1);
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        EXPECT_EQ(factors_A.canonical_id, 2);
        EXPECT_TRUE(factors_A.is_canonical());
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A.raw.symbols[0], 2);
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);
        
        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        EXPECT_EQ(factors_B.canonical_id, 3);
        EXPECT_TRUE(factors_B.is_canonical());
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.raw.symbols.size(), 1);
        EXPECT_EQ(factors_B.raw.symbols[0], 3);
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);

        // AB -> A, B
        const auto& factors_AB = factors[4];
        EXPECT_EQ(factors_AB.id, 4);
        EXPECT_EQ(factors_AB.canonical_id, 4);
        EXPECT_TRUE(factors_AB.is_canonical());
        ASSERT_EQ(factors_AB.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.raw.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.raw.symbols.size(), 2);
        EXPECT_EQ(factors_AB.raw.symbols[0], 2);
        EXPECT_EQ(factors_AB.raw.symbols[1], 3);
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.canonical.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AB.canonical.symbols[1], 3);
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
        EXPECT_EQ(factors_0.canonical_id, 0);
        EXPECT_TRUE(factors_0.is_canonical());
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.raw.symbols.size(), 1);
        EXPECT_EQ(factors_0.raw.symbols[0], 0);

        // ID
        const auto& factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        EXPECT_EQ(factors_I.canonical_id, 1);
        EXPECT_TRUE(factors_I.is_canonical());
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.raw.symbols.size(), 1);
        EXPECT_EQ(factors_I.raw.symbols[0], 1);
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // A
        const auto& factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        EXPECT_EQ(factors_A.canonical_id, 2);
        EXPECT_TRUE(factors_A.is_canonical());
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A.raw.symbols[0], 2);
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);

        // B
        const auto& factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        EXPECT_EQ(factors_B.canonical_id, 3);
        EXPECT_TRUE(factors_B.is_canonical());
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.raw.symbols.size(), 1);
        EXPECT_EQ(factors_B.raw.symbols[0], 3);
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);

        // C
        const auto& factors_C = factors[4];
        EXPECT_EQ(factors_C.id, 4);
        EXPECT_EQ(factors_C.canonical_id, 4);
        EXPECT_TRUE(factors_C.is_canonical());
        ASSERT_EQ(factors_C.raw.sequences.size(), 1);
        EXPECT_EQ(factors_C.raw.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.raw.symbols.size(), 1);
        EXPECT_EQ(factors_C.raw.symbols[0], 4);
        ASSERT_EQ(factors_C.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_C.canonical.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_C.canonical.symbols[0], 4);

        // AB -> AB
        const auto& factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        EXPECT_EQ(factors_AB.canonical_id, 5);
        EXPECT_TRUE(factors_AB.is_canonical());
        ASSERT_EQ(factors_AB.raw.sequences.size(), 1);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.raw.symbols.size(), 1);
        EXPECT_EQ(factors_AB.raw.symbols[0], 5);
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 5);
        
        // AC -> A, C
        const auto& factors_AC = factors[6];
        EXPECT_EQ(factors_AC.id, 6);
        EXPECT_EQ(factors_AC.canonical_id, 6);
        EXPECT_TRUE(factors_AC.is_canonical());
        ASSERT_EQ(factors_AC.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AC.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.raw.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.raw.symbols.size(), 2);
        EXPECT_EQ(factors_AC.raw.symbols[0], 2);
        EXPECT_EQ(factors_AC.raw.symbols[1], 4);
        ASSERT_EQ(factors_AC.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AC.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.canonical.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AC.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AC.canonical.symbols[1], 4);

        // BC -> BC
        const auto& factors_BC = factors[7];
        EXPECT_EQ(factors_BC.id, 7);
        EXPECT_EQ(factors_BC.canonical_id, 7);
        EXPECT_TRUE(factors_BC.is_canonical());
        ASSERT_EQ(factors_BC.raw.sequences.size(), 1);
        EXPECT_EQ(factors_BC.raw.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.raw.symbols.size(), 1);
        EXPECT_EQ(factors_BC.raw.symbols[0], 7);
        ASSERT_EQ(factors_BC.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_BC.canonical.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_BC.canonical.symbols[0], 7);
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

        const std::set<oper_name_t> all_opers{A0, A1, B00, B01, B10, B11, C0, C1};
        ASSERT_EQ(all_opers.size(), 8);


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
        EXPECT_EQ(factors_A0A1.canonical_id, id_A0A1); // <A0A1> is canonical
        EXPECT_TRUE(factors_A0A1.is_canonical());
        ASSERT_EQ(factors_A0A1.raw.sequences.size(), 2);
        EXPECT_EQ(factors_A0A1.raw.sequences[0], OperatorSequence({A0}, context));
        EXPECT_EQ(factors_A0A1.raw.sequences[1], OperatorSequence({A1}, context));
        ASSERT_EQ(factors_A0A1.raw.symbols.size(), 2);
        EXPECT_EQ(factors_A0A1.raw.symbols[0], id_A0);
        EXPECT_EQ(factors_A0A1.raw.symbols[1], id_A1);
        ASSERT_EQ(factors_A0A1.canonical.sequences.size(), 2); // canonical factorization is <A0><A0>
        EXPECT_EQ(factors_A0A1.canonical.sequences[0], OperatorSequence({A0}, context));
        EXPECT_EQ(factors_A0A1.canonical.sequences[1], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0A1.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_A0A1.canonical.symbols[0], id_A0);
        EXPECT_EQ(factors_A0A1.canonical.symbols[1], id_A0);

        // A0 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A0);
        const auto& factors_A0 = factors[id_A0];
        EXPECT_EQ(factors_A0.id, id_A0);
        EXPECT_EQ(factors_A0.canonical_id, id_A0);
        EXPECT_TRUE(factors_A0.is_canonical());
        ASSERT_EQ(factors_A0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A0.raw.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A0.raw.symbols[0], id_A0);
        ASSERT_EQ(factors_A0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A0.canonical.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A0.canonical.symbols[0], id_A0);

        // A1 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A1);
        const auto& factors_A1 = factors[id_A1];
        EXPECT_EQ(factors_A1.id, id_A1);
        EXPECT_EQ(factors_A1.canonical_id, id_A0);
        EXPECT_FALSE(factors_A1.is_canonical());
        ASSERT_EQ(factors_A1.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A1.raw.sequences[0], OperatorSequence({A1}, context));
        ASSERT_EQ(factors_A1.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A1.raw.symbols[0], id_A1);
        ASSERT_EQ(factors_A1.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A1.canonical.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A1.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A1.canonical.symbols[0], id_A0);
    }

    TEST(FactorTable, FromLocalizingMatrix_NewCanonicals) {
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

        const std::set<oper_name_t> all_opers{A0, A1, B00, B01, B10, B11, C0, C1};
        ASSERT_EQ(all_opers.size(), 8);


        const auto& lm = ims.create_localizing_matrix(LocalizingMatrixIndex{context, 0, OperatorSequence{{A1}, context}});


        // Get A0 symbol
        auto A0_ptr = symbols.where(OperatorSequence{{A0}, context});
        ASSERT_NE(A0_ptr, nullptr);
        auto id_A0 = A0_ptr->Id();

        // Get A1 symbol
        auto A1_ptr = symbols.where(OperatorSequence{{A1}, context});
        ASSERT_NE(A1_ptr, nullptr);
        auto id_A1 = A1_ptr->Id();

        // A0 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A0);
        const auto& factors_A0 = factors[id_A0];
        EXPECT_EQ(factors_A0.id, id_A0);
        EXPECT_EQ(factors_A0.canonical_id, id_A0);
        EXPECT_TRUE(factors_A0.is_canonical());
        ASSERT_EQ(factors_A0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A0.raw.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A0.raw.symbols[0], id_A0);
        ASSERT_EQ(factors_A0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A0.canonical.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A0.canonical.symbols[0], id_A0);

        // A1 symbol added, and valid factors
        ASSERT_GE(factors.size(), id_A1);
        const auto& factors_A1 = factors[id_A1];
        EXPECT_EQ(factors_A1.id, id_A1);
        EXPECT_EQ(factors_A1.canonical_id, id_A0);
        EXPECT_FALSE(factors_A1.is_canonical());
        ASSERT_EQ(factors_A1.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A1.raw.sequences[0], OperatorSequence({A1}, context));
        ASSERT_EQ(factors_A1.raw.symbols.size(), 1);
        EXPECT_EQ(factors_A1.raw.symbols[0], id_A1);
        ASSERT_EQ(factors_A1.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A1.canonical.sequences[0], OperatorSequence({A0}, context));
        ASSERT_EQ(factors_A1.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A1.canonical.symbols[0], id_A0);
    }
}