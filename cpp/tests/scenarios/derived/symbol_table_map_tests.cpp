/**
 * implied_map_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "scenarios/derived/symbol_table_map.h"
#include "scenarios/derived/derived_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "../sparse_utils.h"

#include "stm_factories.h"


namespace Moment::Tests {

    using namespace Moment::Derived;

    TEST(Scenarios_Symmetry_SymbolTableMap, Algebraic2to1) {
        using namespace Moment::Algebraic;

        AlgebraicPrecontext apc{2};
        auto amsPtr = std::make_shared<AlgebraicMatrixSystem>(
                AlgebraicContext::FromNameList({"a", "b"})
        );

        auto& ams = *amsPtr;
        const auto& context = ams.Context();
        const auto& symbols = ams.Symbols();
        ams.generate_dictionary(1);
        EXPECT_EQ(symbols.size(), 4); // 0, 1, a, b

        Eigen::SparseMatrix<double> averaging_map = make_sparse(3, {1.0, 0.0, 0.0,
                                                                    0.0, 0.5, 0.5,
                                                                    0.0, 0.5, 0.5});

        DerivedMatrixSystem dms{amsPtr, DirectSparseSTMFactory{std::move(averaging_map)}};
        ASSERT_EQ(dms.Symbols().size(),3); // 0, 1, x

        const SymbolTableMap& stm = dms.map();
        ASSERT_EQ(stm.fwd_size(), 4); // 0, 1, a, b
        ASSERT_EQ(stm.inv_size(), 3); // 0, 1, x
        EXPECT_TRUE(stm.is_monomial_map());
        const symbol_name_t I = 1, a = 2, b = 3, x = 2;

        EXPECT_EQ(stm.inverse(x), SymbolCombo({Monomial{a, 0.5}, Monomial{b, 0.5}}));

        EXPECT_EQ(stm(I), SymbolCombo::Scalar(1.0)); // 1 -> 1
        EXPECT_EQ(stm(a), SymbolCombo({Monomial{x, 1.0}})); // a -> x
        EXPECT_EQ(stm(b), SymbolCombo({Monomial{x, 1.0}})); // b -> x

        EXPECT_EQ(stm(Monomial{I, -5.0}), SymbolCombo({Monomial{I, -5.0}}));
        EXPECT_EQ(stm(Monomial{a, 2.0}), SymbolCombo({Monomial{x, 2.0}}));
        EXPECT_EQ(stm(Monomial{b, 2.0, true}), SymbolCombo({Monomial{x, 2.0}}));

        EXPECT_EQ(stm(SymbolCombo({Monomial{I, 1.0}, Monomial{a, -2.0}})),
                      SymbolCombo({Monomial{I, 1.0}, Monomial{x, -2.0}}));

        EXPECT_EQ(stm(SymbolCombo({Monomial{a, 1.0}, Monomial{b, -2.0}})),
                      SymbolCombo({Monomial{x, -1.0}}));

        EXPECT_EQ(stm(SymbolCombo({Monomial{a, -3.0}})),
                      SymbolCombo({Monomial{x, -3.0}}));

        EXPECT_EQ(stm(SymbolCombo({Monomial{a, 2.0}, Monomial{b, -2.0}})),
                      SymbolCombo::Zero());
    }
}