/**
 * symbol_table_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/matrix/symbol_table.h"
#include "operators/matrix/moment_matrix.h"
#include "operators/matrix/matrix_system.h"
#include "operators/locality/locality_context.h"
#include "operators/locality/locality_matrix_system.h"


namespace NPATK::Tests {

    TEST(SymbolTable, ToSymbol_1Party2Opers) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        auto& context = system.Context();
        const auto& a = context;

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));

        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(matLevel1.Symbols.size(), 7);
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0]}, context})), SymbolExpression(2));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1]}, context})), SymbolExpression(3));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0], a[0]}, context})), SymbolExpression(4));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0], a[1]}, context})), SymbolExpression(5));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1], a[0]}, context})), SymbolExpression(5, true));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1], a[1]}, context})), SymbolExpression(6));

        auto [id2, matLevel2] = system.create_moment_matrix(2);
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0]}, context)), SymbolExpression(2));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1]}, context)), SymbolExpression(3));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0]}, context)),
                  SymbolExpression(4));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1]}, context)),
                  SymbolExpression(5));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0]}, context)),
                  SymbolExpression(5, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1]}, context)),
                  SymbolExpression(6));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0]}, context)),
                  SymbolExpression(7));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1]}, context)),
                  SymbolExpression(8));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0]}, context)),
                  SymbolExpression(8, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0]}, context)),
                  SymbolExpression(9));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1]}, context)),
                  SymbolExpression(10));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0]}, context)),
                  SymbolExpression(10, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1]}, context)),
                  SymbolExpression(11));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1]}, context)),
                  SymbolExpression(12));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0], a[0]}, context)),
                  SymbolExpression(13));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0], a[1]}, context)),
                  SymbolExpression(14));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0], a[0]}, context)),
                  SymbolExpression(14, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1], a[0]}, context)),
                  SymbolExpression(15));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0], a[0]}, context)),
                  SymbolExpression(15, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1], a[1]}, context)),
                  SymbolExpression(16));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0], a[0]}, context)),
                  SymbolExpression(16, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0], a[1]} , context)),
                  SymbolExpression(17));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0], a[1]} , context)),
                  SymbolExpression(18));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1], a[0]} , context)),
                  SymbolExpression(18, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1], a[1]} , context)),
                  SymbolExpression(19));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0], a[1]} , context)),
                  SymbolExpression(19, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1], a[0]}, context)),
                  SymbolExpression(20));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1], a[1]} , context)),
                  SymbolExpression(21));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1], a[0]} , context)),
                  SymbolExpression(21, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1], a[1]} , context)),
                  SymbolExpression(22));
    };

    TEST(SymbolTable, ToSymbol_2Party1Opers) {
        // Two parties, each with one operator
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        auto [id0, matLevel0] = system.create_moment_matrix(0); //0 1
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));

        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a b ab
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{alice[0]}, context})), SymbolExpression(2));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{bob[0]}, context})), SymbolExpression(3));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{alice[0], bob[0]}, context})), SymbolExpression(4));

        auto [id2, matLevel2] = system.create_moment_matrix(2); // as above
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Zero(context)), SymbolExpression(0));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Identity(context)), SymbolExpression(1));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{alice[0]}, context})), SymbolExpression(2));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{bob[0]}, context})), SymbolExpression(3));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{alice[0], bob[0]}, context})), SymbolExpression(4));


    }

    TEST(SymbolTable, Enumerate_1Party2Opers) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
        ASSERT_EQ(symbols.RealSymbolIds().size(), 6);
        ASSERT_EQ(symbols.ImaginarySymbolIds().size(), 1); // just a0a1

        for (size_t i = 0; i < 6; ++i) {
            EXPECT_EQ(symbols.RealSymbolIds()[i], i+1) << i;
            auto [re_id, im_id] = symbols[i+1].basis_key();
            EXPECT_EQ(re_id, i) << i;
        }

        EXPECT_EQ(symbols.ImaginarySymbolIds()[0], 5);
        auto [re_id, im_id] = symbols[5].basis_key();
        EXPECT_EQ(im_id, 0);

    };

    TEST(SymbolTable, SMP_BasisKey) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1

        auto basis_key = matLevel1.SMP().BasisKey();
        ASSERT_EQ(basis_key.size(), 7);

        auto f0Iter = basis_key.find(0);
        ASSERT_NE(f0Iter, basis_key.end());
        EXPECT_EQ(f0Iter->second.first, -1);
        EXPECT_EQ(f0Iter->second.second, -1);
        
        auto f1Iter = basis_key.find(1);
        ASSERT_NE(f1Iter, basis_key.end());
        EXPECT_EQ(f1Iter->second.first, 0);
        EXPECT_EQ(f1Iter->second.second, -1);
        
        auto f2Iter = basis_key.find(2);
        ASSERT_NE(f2Iter, basis_key.end());
        EXPECT_EQ(f2Iter->second.first, 1);
        EXPECT_EQ(f2Iter->second.second, -1);
        
        auto f3Iter = basis_key.find(3);
        ASSERT_NE(f3Iter, basis_key.end());
        EXPECT_EQ(f3Iter->second.first, 2);
        EXPECT_EQ(f3Iter->second.second, -1);
        
        auto f4Iter = basis_key.find(4);
        ASSERT_NE(f4Iter, basis_key.end());
        EXPECT_EQ(f4Iter->second.first, 3);
        EXPECT_EQ(f4Iter->second.second, -1);
        
        auto f5Iter = basis_key.find(5);
        ASSERT_NE(f5Iter, basis_key.end());
        EXPECT_EQ(f5Iter->second.first, 4);
        EXPECT_EQ(f5Iter->second.second, 0);
        
        auto f6Iter = basis_key.find(6);
        ASSERT_NE(f6Iter, basis_key.end());
        EXPECT_EQ(f6Iter->second.first, 5);
        EXPECT_EQ(f6Iter->second.second, -1);
    };

}