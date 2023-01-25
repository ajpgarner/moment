/**
 * extension_suggester_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/extension_suggester.h"

#include "../../symbolic/symbolic_matrix_helpers.h"

#include <sstream>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_ExtensionSuggester, Pair_Unlinked) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        ExtensionSuggester suggester{ims.InflationContext(), symbols, factors};

        ims.create_moment_matrix(1);
        const auto& base_MM = ims.MomentMatrix(1);

        ASSERT_EQ(symbols.size(), 5);
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A = find_or_fail(symbols, OperatorSequence{{0}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{1}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{0, 1}, ims.Context()});
        std::set all_symbs{id_0, id_e, id_A, id_B, id_AB};
        ASSERT_EQ(all_symbs.size(), 5);

        auto required = suggester.nonfundamental_symbols(base_MM);
        EXPECT_EQ(required.count(), 1);
        EXPECT_TRUE(required.test(id_AB));

        auto suggested = suggester(base_MM);
        EXPECT_EQ(suggested.size(), 1);
        ASSERT_GE(suggested.size(), 1);
        EXPECT_EQ(*suggested.begin(), id_A);
    }

    TEST(Scenarios_Inflation_ExtensionSuggester, Pair_LinkedPairFactorCV) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2, 0}, {{0, 1}}}, 2)};
        auto& context = ims.InflationContext();
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        ExtensionSuggester suggester{context, symbols, factors};

        ims.create_moment_matrix(2);
        const auto& base_MM = ims.MomentMatrix(2);

        ASSERT_EQ(context.Observables().size(), 3);
        auto op_A = context.Observables()[0].operator_offset;
        auto op_B = context.Observables()[1].operator_offset;
        auto op_C = context.Observables()[2].operator_offset;

        auto id_A = find_or_fail(symbols, OperatorSequence{{op_A}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{op_B}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{op_A, op_B}, ims.Context()});
        auto id_CC = find_or_fail(symbols, OperatorSequence{{op_C, op_C}, ims.Context()});
        auto id_CCC = find_or_fail(symbols, OperatorSequence{{op_C, op_C, op_C}, ims.Context()});

        auto suggested = suggester(base_MM);
        // S2, S3, S6, S11, S21
        EXPECT_EQ(suggested.size(), 5);
        EXPECT_TRUE(suggested.contains(id_A));
        EXPECT_TRUE(suggested.contains(id_B));
        EXPECT_TRUE(suggested.contains(id_AB));
        EXPECT_TRUE(suggested.contains(id_CC));
        EXPECT_TRUE(suggested.contains(id_CCC));

    }
}