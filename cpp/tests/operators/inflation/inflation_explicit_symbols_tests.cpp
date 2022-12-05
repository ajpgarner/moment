/**
 * inflation_explicit_symbols_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"
#include "operators/inflation/inflation_explicit_symbols.h"

#include "operators/matrix/moment_matrix.h"

namespace NPATK::Tests {

    TEST(Operators_Inflation_ExplicitSymbols, W) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 3}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        const auto a0 = context.Observables()[0].operator_offset;
        const auto b0 = context.Observables()[1].operator_offset;
        const auto c0 = context.Observables()[2].operator_offset;
        const auto c1 = context.Observables()[2].operator_offset + 1;

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
}