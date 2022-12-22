/**
 * inflation_implicit_symbols_tests.cpp.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_implicit_symbols.h"

#include "matrix/moment_matrix.h"

#include "../implicit_symbol_test_helpers.h"

namespace Moment::Tests {

    TEST(Operators_Inflation_ImplicitSymbols, Empty) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        EXPECT_EQ(implSym.MaxSequenceLength, 0);
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 1);

        const auto& one = implSym.Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        SymbolCombo oneCombo{{1,1.0}};
        EXPECT_EQ(one.expression, oneCombo);

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);
        EXPECT_EQ(&getOne[0], &one);
    }

    TEST(Operators_Inflation_ImplicitSymbols, Singleton) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1); // should be [[1 A]; [A A]]
        const auto& implSym = ims.ImplicitSymbolTable();

        EXPECT_EQ(implSym.MaxSequenceLength, 1);
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 3); // e, a0, a1

        const auto& one = implSym.Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        SymbolCombo oneCombo{{1,1.0}};
        EXPECT_EQ(one.expression, oneCombo);

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);
        EXPECT_EQ(&getOne[0], &one);

        const auto& getA = implSym.get(std::vector<OVIndex>{{0, 0}});
        test2Mmt(getA, 1, 2);
    }

    TEST(Operators_Inflation_ImplicitSymbols, Singleton_Cloned) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        ASSERT_EQ(implSym.MaxSequenceLength, 2); // now we have A0A1 too
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 7); // e, a0 [2], a0a1 [4]

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);

        const auto& getA = implSym.get(std::vector<OVIndex>{{0, 0}});
        test2Mmt(getA, 1, 2, "A0");

        const auto& getAprime = implSym.get(std::vector<OVIndex>{{0, 1}});
        test2Mmt(getAprime, 1, 2, "A1");

        const auto& getAAprime = implSym.get(std::vector<OVIndex>{{0, 0}, {0, 1}});
        test22JoinMmt(getAAprime, 1, 2, 2, 3, "A0A1");
    }

    TEST(Operators_Inflation_ImplicitSymbols, Pair_OneCV) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 0}, {{0, 1}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

    }

    TEST(Operators_Inflation_ImplicitSymbols, Block) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        ASSERT_EQ(implSym.MaxSequenceLength, 2); // now we have A0A1 too
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 7); // e, a0 [2], a0a1 [4]

        auto eBlock = implSym.Block(0);
        ASSERT_EQ(eBlock.size(), 1);
        EXPECT_EQ(eBlock[0].symbol_id, 1);

        auto a0Block = implSym.Block(1);
        ASSERT_EQ(a0Block.size(), 2);
        EXPECT_EQ(a0Block[0].symbol_id, 2);
        EXPECT_EQ(a0Block[1].symbol_id, -1);





    }
}