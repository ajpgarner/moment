/**
 * imported_matrix_system_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "matrix/symbolic_matrix.h"
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

    TEST(Scenarios_Imported_ImportedMatrixSystem, Construct_TwoByTwo) {
        ImportedMatrixSystem ims{};
        const auto& context = ims.Context();
        const auto& symbols = ims.Symbols();

        std::vector<SymbolExpression> rawData{SymbolExpression{2}, SymbolExpression{3},
                                              SymbolExpression{3}, SymbolExpression{4}};
        auto raw_mat_ptr = std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(rawData));

        size_t index = ims.import_matrix(std::move(raw_mat_ptr));
        EXPECT_EQ(index, 0);
        ASSERT_EQ(ims.size(), 1);
        ASSERT_EQ(symbols.size(), 5);

        const auto& imported_matrix = ims[0];
        EXPECT_EQ(&imported_matrix.Symbols, &symbols);
        EXPECT_EQ(&imported_matrix.context, &context);
        ASSERT_EQ(imported_matrix.Dimension(), 2);
        EXPECT_EQ(imported_matrix.SymbolMatrix[0][0], SymbolExpression{2});
        EXPECT_EQ(imported_matrix.SymbolMatrix[0][1], SymbolExpression{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix[1][0], SymbolExpression{3});
        EXPECT_EQ(imported_matrix.SymbolMatrix[1][1], SymbolExpression{4});

    }


}
