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

        ExtensionSuggester suggester{symbols, factors};

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
}