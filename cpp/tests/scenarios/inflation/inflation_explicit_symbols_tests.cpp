/**
 * inflation_explicit_symbols_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_explicit_symbols.h"

#include "matrix/moment_matrix.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;


    TEST(Scenarios_Inflation_ExplicitSymbols, W) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 3}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        oper_name_t a0 = context.Observables()[0].operator_offset;
        oper_name_t b0 = context.Observables()[1].operator_offset;
        oper_name_t c0 = context.Observables()[2].operator_offset;
        oper_name_t c1 = context.Observables()[2].operator_offset + 1;

        const auto* ePtr = symbols.where(OperatorSequence::Identity(context));

        const auto* a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        const auto* b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        const auto* c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        const auto* c1Ptr = symbols.where(OperatorSequence{{c1}, context});

        const auto* a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        const auto* a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        const auto* a0c1Ptr = symbols.where(OperatorSequence{{a0, c1}, context});
        const auto* b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        const auto* b0c1Ptr = symbols.where(OperatorSequence{{b0, c1}, context});

        const auto* a0b0c0Ptr = symbols.where(OperatorSequence{{a0, b0, c0}, context});
        const auto* a0b0c1Ptr = symbols.where(OperatorSequence{{a0, b0, c1}, context});

        ASSERT_NE(ePtr, nullptr);

        ASSERT_NE(a0Ptr, nullptr);
        ASSERT_NE(b0Ptr, nullptr);
        ASSERT_NE(c0Ptr, nullptr);
        ASSERT_NE(c1Ptr, nullptr);

        ASSERT_NE(a0b0Ptr, nullptr);
        ASSERT_NE(a0c0Ptr, nullptr);
        ASSERT_NE(a0c1Ptr, nullptr);
        ASSERT_NE(b0c0Ptr, nullptr);
        ASSERT_NE(b0c1Ptr, nullptr);

        ASSERT_NE(a0b0c0Ptr, nullptr);
        ASSERT_NE(a0b0c1Ptr, nullptr);

        const auto& explicit_symbols = ims.ExplicitSymbolTable();

        // I
        auto s_e = explicit_symbols.get(std::vector<OVIndex>{});
        ASSERT_EQ(s_e.size(), 1);
        EXPECT_EQ(s_e[0].symbol_id, ePtr->Id());

        // A
        auto s_a = explicit_symbols.get(std::vector{OVIndex{0, 0}});
        ASSERT_EQ(s_a.size(), 1);
        EXPECT_EQ(s_a[0].symbol_id, a0Ptr->Id());

        // B
        auto s_b = explicit_symbols.get(std::vector{OVIndex{1, 0}});
        ASSERT_EQ(s_b.size(), 1);
        EXPECT_EQ(s_b[0].symbol_id, b0Ptr->Id()) << symbols;

        // C
        auto s_c = explicit_symbols.get(std::vector{OVIndex{2, 0}});
        ASSERT_EQ(s_c.size(), 2);
        EXPECT_EQ(s_c[0].symbol_id, c0Ptr->Id());
        EXPECT_EQ(s_c[1].symbol_id, c1Ptr->Id());

        // AB
        auto s_ab = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 0}});
        ASSERT_EQ(s_ab.size(), 1);
        EXPECT_EQ(s_ab[0].symbol_id, a0b0Ptr->Id());

        // AC
        auto s_ac = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{2, 0}});
        ASSERT_EQ(s_ac.size(), 2);
        EXPECT_EQ(s_ac[0].symbol_id, a0c0Ptr->Id());
        EXPECT_EQ(s_ac[1].symbol_id, a0c1Ptr->Id());

        // BC
        auto s_bc = explicit_symbols.get(std::vector{OVIndex{1, 0}, OVIndex{2, 0}});
        ASSERT_EQ(s_bc.size(), 2);
        EXPECT_EQ(s_bc[0].symbol_id, b0c0Ptr->Id());
        EXPECT_EQ(s_bc[1].symbol_id, b0c1Ptr->Id());

        // ABC
        auto s_abc = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 0}, OVIndex{2, 0}});
        ASSERT_EQ(s_abc.size(), 2);
        EXPECT_EQ(s_abc[0].symbol_id, a0b0c0Ptr->Id());
        EXPECT_EQ(s_abc[1].symbol_id, a0b0c1Ptr->Id());
    }

    TEST(Scenarios_Inflation_ExplicitSymbols, SingletonPair) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 0}, {{0, 1}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        const auto& A = context.Observables()[0];
        const auto& B = context.Observables()[1];
        const auto& C = context.Observables()[2];

        ASSERT_EQ(A.variants.size(), 2);
        ASSERT_EQ(B.variants.size(), 2);
        ASSERT_EQ(C.variants.size(), 1);
        const auto& A0 = A.variants[0];
        const auto& A1 = A.variants[1];
        const auto& B0 = B.variants[0];
        const auto& B1 = B.variants[1];
        const auto& C0 = C.variants[0];

        const auto a0 = A0.operator_offset;
        const auto a1 = A1.operator_offset;
        const auto b0 = B0.operator_offset;
        const auto b1 = B1.operator_offset;
        const auto c0 = C0.operator_offset;

        std::set all_ids{a0,a1,b0,b1,c0};
        ASSERT_EQ(all_ids.size(), 5);

        const auto* ePtr = symbols.where(OperatorSequence::Identity(context));
        ASSERT_NE(ePtr, nullptr);

        const auto* a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        ASSERT_NE(a0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{a1}, context}), nullptr); // Symmetric to a0
        const auto* b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        ASSERT_NE(b0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{b1}, context}), nullptr); // Symmetric to b0
        const auto* c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        ASSERT_NE(c0Ptr, nullptr);

        const auto* a0a1Ptr = symbols.where(OperatorSequence{{a0, a1}, context});
        ASSERT_NE(a0a1Ptr, nullptr);
        const auto* a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        ASSERT_NE(a0b0Ptr, nullptr);
        const auto* a0b1Ptr = symbols.where(OperatorSequence{{a0, b1}, context});
        ASSERT_NE(a0b1Ptr, nullptr);
        const auto* a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        ASSERT_NE(a0c0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b0}, context}), nullptr);// Symmetric to a0b1
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b1}, context}), nullptr); // Symmetric to a0b0
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, c0}, context}), nullptr); // Symmetric to a0c0
        const auto* b0b1Ptr = symbols.where(OperatorSequence{{b0, b1}, context});
        ASSERT_NE(b0b1Ptr, nullptr);
        const auto* b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        ASSERT_NE(b0c0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{b1, c0}, context}), nullptr); // Symmetric to b0c0

        const auto& explicit_symbols = ims.ExplicitSymbolTable();

        // I
        auto s_e = explicit_symbols.get(std::vector<OVIndex>{});
        ASSERT_EQ(s_e.size(), 1);
        EXPECT_EQ(s_e[0].symbol_id, ePtr->Id());

        // A0
        auto s_a0 = explicit_symbols.get(std::vector{OVIndex{0, 0}});
        ASSERT_EQ(s_a0.size(), 1);
        EXPECT_EQ(s_a0[0].symbol_id, a0Ptr->Id());

        // A1
        auto s_a1 = explicit_symbols.get(std::vector{OVIndex{0, 1}});
        ASSERT_EQ(s_a1.size(), 1);
        EXPECT_EQ(s_a1[0].symbol_id, a0Ptr->Id());

        // B0
        auto s_b0 = explicit_symbols.get(std::vector{OVIndex{1, 0}});
        ASSERT_EQ(s_b0.size(), 1);
        EXPECT_EQ(s_b0[0].symbol_id, b0Ptr->Id());

        // B1
        auto s_b1 = explicit_symbols.get(std::vector{OVIndex{1, 1}});
        ASSERT_EQ(s_b1.size(), 1);
        EXPECT_EQ(s_b1[0].symbol_id, b0Ptr->Id());

        // C
        auto s_c = explicit_symbols.get(std::vector{OVIndex{2, 0}});
        ASSERT_EQ(s_c.size(), 1);
        EXPECT_EQ(s_c[0].symbol_id, c0Ptr->Id());

        // A0A1
        auto s_a0a1 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{0, 1}});
        ASSERT_EQ(s_a0a1.size(), 1);
        EXPECT_EQ(s_a0a1[0].symbol_id, a0a1Ptr->Id());

        // A0B0
        auto s_a0b0 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 0}});
        ASSERT_EQ(s_a0b0.size(), 1);
        EXPECT_EQ(s_a0b0[0].symbol_id, a0b0Ptr->Id());

        // A0B1
        auto s_a0b1 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 1}});
        ASSERT_EQ(s_a0b1.size(), 1);
        EXPECT_EQ(s_a0b1[0].symbol_id, a0b1Ptr->Id());

        // A0C0
        auto s_a0c0 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{2, 0}});
        ASSERT_EQ(s_a0c0.size(), 1);
        EXPECT_EQ(s_a0c0[0].symbol_id, a0c0Ptr->Id());

        // A1B0
        auto s_a1b0 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{1, 0}});
        ASSERT_EQ(s_a1b0.size(), 1);
        EXPECT_EQ(s_a1b0[0].symbol_id, a0b1Ptr->Id()); // symmetric

        // A1B1
        auto s_a1b1 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{1, 1}});
        ASSERT_EQ(s_a1b1.size(), 1);
        EXPECT_EQ(s_a1b1[0].symbol_id, a0b0Ptr->Id()); // symmetric

        // A1C0
        auto s_a1c0 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{2, 0}});
        ASSERT_EQ(s_a1c0.size(), 1);
        EXPECT_EQ(s_a1c0[0].symbol_id, a0c0Ptr->Id()); // symmetric

        // B0B1
        auto s_b0b1 = explicit_symbols.get(std::vector{OVIndex{1, 0}, OVIndex{1, 1}});
        ASSERT_EQ(s_b0b1.size(), 1);
        EXPECT_EQ(s_b0b1[0].symbol_id, b0b1Ptr->Id());

        // B0C0
        auto s_b0c0 = explicit_symbols.get(std::vector{OVIndex{1, 0}, OVIndex{2, 0}});
        ASSERT_EQ(s_b0c0.size(), 1);
        EXPECT_EQ(s_b0c0[0].symbol_id, b0c0Ptr->Id());

        // B1C0
        auto s_b1c0 = explicit_symbols.get(std::vector{OVIndex{1, 1}, OVIndex{2, 0}});
        ASSERT_EQ(s_b1c0.size(), 1);
        EXPECT_EQ(s_b1c0[0].symbol_id, b0c0Ptr->Id()); // symmetric
    }
}