/**
 * symbol_table_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix_system.h"

#include "matrix/moment_matrix.h"

#include "scenarios/context.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "symbolic/symbol_table.h"

namespace Moment::Tests {

    TEST(Symbolic_SymbolTable, ToSymbol_1Party2Opers) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        auto& context = system.Context();
        std::vector<oper_name_t> a{0, 1};

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));

        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(matLevel1.Symbols.size(), 7);
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0]}, context})), Monomial(2));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1]}, context})), Monomial(3));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0], a[0]}, context})), Monomial(4));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[0], a[1]}, context})), Monomial(5));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1], a[0]}, context})), Monomial(5, true));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{a[1], a[1]}, context})), Monomial(6));

        auto [id2, matLevel2] = system.create_moment_matrix(2);
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0]}, context)), Monomial(2));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1]}, context)), Monomial(3));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0]}, context)),
                  Monomial(4));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1]}, context)),
                  Monomial(5));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0]}, context)),
                  Monomial(5, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1]}, context)),
                  Monomial(6));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0]}, context)),
                  Monomial(7));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1]}, context)),
                  Monomial(8));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0]}, context)),
                  Monomial(8, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0]}, context)),
                  Monomial(9));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1]}, context)),
                  Monomial(10));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0]}, context)),
                  Monomial(10, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1]}, context)),
                  Monomial(11));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1]}, context)),
                  Monomial(12));

        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0], a[0]}, context)),
                  Monomial(13));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[0], a[1]}, context)),
                  Monomial(14));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0], a[0]}, context)),
                  Monomial(14, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1], a[0]}, context)),
                  Monomial(15));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0], a[0]}, context)),
                  Monomial(15, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[0], a[1], a[1]}, context)),
                  Monomial(16));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0], a[0]}, context)),
                  Monomial(16, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[0], a[1]} , context)),
                  Monomial(17));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[0], a[1]} , context)),
                  Monomial(18));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1], a[0]} , context)),
                  Monomial(18, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[0], a[1], a[1]} , context)),
                  Monomial(19));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[0], a[1]} , context)),
                  Monomial(19, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1], a[0]}, context)),
                  Monomial(20));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[0], a[1], a[1], a[1]} , context)),
                  Monomial(21));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1], a[0]} , context)),
                  Monomial(21, true));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence({a[1], a[1], a[1], a[1]} , context)),
                  Monomial(22));
    };

    TEST(Symbolic_SymbolTable, ToSymbol_2Party1Opers) {
        using namespace Moment::Locality;

        // Two parties, each with one operator
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        auto [id0, matLevel0] = system.create_moment_matrix(0); //0 1
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel0.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));

        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a b ab
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel1.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{alice[0]}, context})), Monomial(2));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{bob[0]}, context})), Monomial(3));
        EXPECT_EQ(matLevel1.Symbols.to_symbol((OperatorSequence{{alice[0], bob[0]}, context})), Monomial(4));

        auto [id2, matLevel2] = system.create_moment_matrix(2); // as above
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Zero(context)), Monomial(0));
        EXPECT_EQ(matLevel2.Symbols.to_symbol(OperatorSequence::Identity(context)), Monomial(1));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{alice[0]}, context})), Monomial(2));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{bob[0]}, context})), Monomial(3));
        EXPECT_EQ(matLevel2.Symbols.to_symbol((OperatorSequence{{alice[0], bob[0]}, context})), Monomial(4));


    }

    TEST(Symbolic_SymbolTable, Enumerate_1Party2Opers) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7) << symbols; // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
        ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6) << symbols;
        ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1) << symbols; // just a0a1

        for (size_t i = 0; i < 6; ++i) {
            EXPECT_EQ(symbols.Basis.RealSymbols()[i], i+1) << i;
            auto [re_id, im_id] = symbols[i+1].basis_key();
            EXPECT_EQ(re_id, i) << i;
        }

        EXPECT_EQ(symbols.Basis.ImaginarySymbols()[0], 5);
        auto [re_id, im_id] = symbols[5].basis_key();
        EXPECT_EQ(im_id, 0);

    };

    TEST(Symbolic_SymbolTable, SMP_BasisKey) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1

        auto basis_key = matLevel1.SMP().BasisKey();
        ASSERT_EQ(basis_key.size(), 6) << symbols;

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

    TEST(Symbolic_SymbolTable, SMP_CrossList) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();

        auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1

        // [0,-1]; [1, -1]; [2, -1]; [3, -1]; [4, 0]; [5, -1]
        EXPECT_EQ(symbols.Basis.imaginary_from_real(0), -1);
        EXPECT_EQ(symbols.Basis.imaginary_from_real(1), -1);
        EXPECT_EQ(symbols.Basis.imaginary_from_real(2), -1);
        EXPECT_EQ(symbols.Basis.imaginary_from_real(3), -1);
        EXPECT_EQ(symbols.Basis.imaginary_from_real(4), 0);
        EXPECT_EQ(symbols.Basis.imaginary_from_real(5), -1);

        EXPECT_EQ(symbols.Basis.real_from_imaginary(0), 4);
    };


    TEST(Symbolic_SymbolTable, FillToWordLength) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        auto& context = system.Context();
        auto& symbols = system.Symbols();
        ASSERT_EQ(symbols.size(), 2); // 0 & 1

        auto [totalA, addedA] = symbols.fill_to_word_length(1); // Should add a & b
        ASSERT_EQ(symbols.size(), 4) << symbols;
        ASSERT_EQ(totalA, 3) << symbols; // e, a, b
        ASSERT_EQ(addedA, 2) << symbols; // a, b
        EXPECT_EQ(symbols[2].sequence(), (OperatorSequence{{0}, context})) << symbols[2];
        EXPECT_TRUE(symbols[2].is_hermitian()) << symbols[2];
        EXPECT_EQ(symbols[3].sequence(), (OperatorSequence{{1}, context})) << symbols[3];
        EXPECT_TRUE(symbols[3].is_hermitian()) << symbols[3];

        auto [totalB, addedB] = symbols.fill_to_word_length(2); // Should add: e, a, b, aa, ab, (ba=(ab*)), bb
        ASSERT_EQ(symbols.size(), 7) << symbols;
        ASSERT_EQ(totalB, 7) << symbols; // e, a, b, aa, ab, (ba), bb
        ASSERT_EQ(addedB, 3) << symbols; // aa, ab, bb
        EXPECT_EQ(symbols[4].sequence(), (OperatorSequence{{0, 0}, context})) << symbols[4];
        EXPECT_TRUE(symbols[4].is_hermitian()) << symbols[4];
        EXPECT_EQ(symbols[5].sequence(), (OperatorSequence{{0, 1}, context})) << symbols[5];
        EXPECT_EQ(symbols[5].sequence_conj(), (OperatorSequence{{1, 0}, context})) << symbols[5];
        EXPECT_FALSE(symbols[5].is_hermitian()) << symbols[5];
        EXPECT_EQ(symbols[6].sequence(), (OperatorSequence{{1, 1}, context})) << symbols[6];
        EXPECT_TRUE(symbols[6].is_hermitian()) << symbols[6];


     }

     TEST(Symbolic_SymbolTable, FillToWordLength_Redundant) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        auto& context = system.Context();
        auto& symbols = system.Symbols();
        ASSERT_EQ(symbols.size(), 2); // 0 & 1

        const auto& mm1 = system.create_moment_matrix(1);
        ASSERT_EQ(symbols.size(), 7) << symbols;

        auto [totalA, addedA] = symbols.fill_to_word_length(1); // Should add a & b
        ASSERT_EQ(symbols.size(), 7) << symbols;
        ASSERT_EQ(totalA, 3) << symbols; // e, a, b
        ASSERT_EQ(addedA, 0) << symbols; // a, b

        auto [totalB, addedB] = symbols.fill_to_word_length(2); // Should add: e, a, b, aa, ab, (ba=(ab*)), bb
        ASSERT_EQ(symbols.size(), 7) << symbols;
        ASSERT_EQ(totalB, 7) << symbols; // e, a, b, aa, ab, (ba), bb
        ASSERT_EQ(addedB, 0) << symbols; // aa, ab, bb

     }

}
