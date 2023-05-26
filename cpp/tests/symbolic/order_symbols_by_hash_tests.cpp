/**
 * order_symbols_by_hash_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "symbolic/order_symbols_by_hash.h"

namespace Moment::Tests {

    TEST(Symbolic_OrderByHash, Comparator) {

        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        CompareByOpHash comparator{symbols};

        EXPECT_TRUE(comparator(Monomial{1}, Monomial{2}));
        EXPECT_FALSE(comparator(Monomial{2}, Monomial{1}));

        SymbolCombo combo({Monomial{1, 1.0}, Monomial{2, 1.0}, Monomial{5, 2.0, true}},
                          symbols, comparator);

        ASSERT_EQ(combo.size(), 3);
        EXPECT_EQ(combo[0], Monomial(1, 1.0));
        EXPECT_EQ(combo[1], Monomial(2, 1.0));
        EXPECT_EQ(combo[2], Monomial(5, 2.0, true));
        EXPECT_FALSE(combo.is_hermitian(symbols));
        EXPECT_EQ(combo.first_id(), 1);
        EXPECT_EQ(combo.last_id(), 5);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }

    TEST(Symbolic_OrderByHash, Comparator_NontrivialHermitian) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();
        ASSERT_EQ(context.size(), 2);
        ams.generate_dictionary(2);
        ASSERT_EQ(symbols.size(), 7);  // 0, 1, a, b, aa, ab, (ba), bb

        CompareByOpHash comparator{symbols};

        SymbolCombo combo({Monomial{5, 2.0, false}, Monomial{5, 2.0, true}},
                          symbols, comparator);

        ASSERT_EQ(combo.size(), 2);
        EXPECT_EQ(combo[0], Monomial(5, 2.0, false));
        EXPECT_EQ(combo[1], Monomial(5, 2.0, true));
        EXPECT_TRUE(combo.is_hermitian(symbols));
        EXPECT_EQ(combo.first_id(), 5);
        EXPECT_EQ(combo.last_id(), 5);

        auto cc_combo = combo.conjugate(symbols);
        EXPECT_TRUE(combo.is_conjugate(symbols, cc_combo));
        EXPECT_TRUE(cc_combo.is_conjugate(symbols, combo));
    }
}