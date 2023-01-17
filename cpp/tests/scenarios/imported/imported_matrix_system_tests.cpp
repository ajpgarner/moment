/**
 * imported_matrix_system_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "symbolic/symbol_table.h"
#include "scenarios/imported/imported_matrix_system.h"

namespace Moment::Tests {
    using namespace Moment::Imported;

    TEST(Scenarios_Imported_ImportedMatrixSystem, Construct_Empty) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        EXPECT_EQ(context.size(), 0);

        const auto& symbols = ims.Symbols();
        ASSERT_EQ(symbols.size(), 2);
        EXPECT_EQ(symbols[0].Id(), 0);
        EXPECT_EQ(symbols[1].Id(), 1);
    }


}
