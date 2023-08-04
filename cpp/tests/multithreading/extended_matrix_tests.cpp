/**
 * extended_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix_system/matrix_system.h"
#include "matrix/operator_matrix/localizing_matrix.h"
#include "scenarios/context.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/extension_suggester.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/multithreading/temporary_symbols_and_factors.h"

#include "symbolic/symbol_table.h"

namespace Moment::Tests {
    using namespace Moment::Multithreading;
    using namespace Moment::Inflation;

    TEST(Multithreading_ExtendedMatrix, TemporarySymbolsAndFactors) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2},
                                                                                   {{0, 1}, {1, 2}, {0, 2}}},
                                                                     2)};
        const auto& context = ims.InflationContext();
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();
        const auto& mm = ims.MomentMatrix(1);

        ASSERT_EQ(context.Observables().size(), 3);
        const auto& A = context.Observables()[0];
        ASSERT_EQ(A.variant_count, 4);
        const auto& A00 = A.variants[0];
        const auto& A10 = A.variants[1];
        const auto& A01 = A.variants[2];
        const auto& A11 = A.variants[3];

        const auto& B = context.Observables()[1];
        ASSERT_EQ(B.variant_count, 4);
        const auto& B00 = B.variants[0];
        const auto& B10 = B.variants[1];
        const auto& B01 = B.variants[2];
        const auto& B11 = B.variants[3];

        const auto& C = context.Observables()[2];
        ASSERT_EQ(C.variant_count, 4);
        const auto& C00 = C.variants[0];
        const auto& C10 = C.variants[1];
        const auto& C01 = C.variants[2];
        const auto& C11 = C.variants[3];


        EXPECT_EQ(symbols.size(), 20);
        EXPECT_EQ(factors.size(), 20);

        // Check factors exist in plain table
        const OperatorSequence seq_A00{{A00.operator_offset}, context};
        const auto where_A00 = symbols.where(seq_A00);
        ASSERT_TRUE(where_A00.found());
        
        const OperatorSequence seq_C11{{C11.operator_offset}, context};
        const auto where_C11 = symbols.where(seq_C11);
        ASSERT_TRUE(where_C11.found());
        EXPECT_TRUE(where_C11.is_aliased); // alias of A00

        const OperatorSequence seq_A00C11{{A00.operator_offset, C11.operator_offset}, context};
        const auto where_A00C11 = symbols.where(seq_A00C11);
        ASSERT_TRUE(where_A00C11.found());

        auto factors_of_A00C11 = context.factorize(seq_A00C11);
        ASSERT_EQ(factors_of_A00C11.size(), 2);
        EXPECT_EQ(factors_of_A00C11[0], seq_A00);
        EXPECT_EQ(factors_of_A00C11[1], seq_C11);

        const auto& reference_factors = factors[where_A00C11->Id()].canonical.symbols;

        TemporarySymbolsAndFactors tsaf{symbols, factors};
        const auto& test_factors = tsaf.find_factors_by_symbol_id(where_A00C11->Id());
        EXPECT_EQ(test_factors, reference_factors);

        auto not_new = tsaf.find_or_register_factors(reference_factors);
        EXPECT_EQ(not_new, where_A00C11->Id());
        EXPECT_EQ(symbols.size(), 20);
        EXPECT_EQ(tsaf.additional_symbol_count(), 0);

        const std::vector<symbol_name_t> new_factor_string{where_A00->Id(), where_A00->Id(), where_A00->Id()};
        EXPECT_FALSE(factors.find_index_by_factors(new_factor_string).has_value());

        auto symbol_id = tsaf.find_or_register_factors(new_factor_string);
        EXPECT_EQ(symbol_id, 20);
        EXPECT_EQ(tsaf.additional_symbol_count(), 1);

        EXPECT_EQ(symbols.size(), 20);
        EXPECT_EQ(factors.size(), 20);

        tsaf.register_new_symbols_and_factors();
        ASSERT_EQ(symbols.size(), 21);
        ASSERT_EQ(factors.size(), 21);
    }


    TEST(Multithreading_ExtendedMatrix, ThreeOutcomeTriangle_Multithread) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{3, 3, 3},
                                                                                   {{0, 1}, {1, 2}, {0, 2}}},
                                                                     2)};
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();
        const auto& factors = ims.Factors();

        ASSERT_EQ(context.Observables().size(), 3);
        const auto& A = context.Observables()[0];
        ASSERT_EQ(A.variant_count, 4);
        const auto& A00 = A.variants[0];
        const auto& A10 = A.variants[1];
        const auto& A01 = A.variants[2];
        const auto& A11 = A.variants[3];

        const auto& B = context.Observables()[1];
        ASSERT_EQ(B.variant_count, 4);
        const auto& B00 = B.variants[0];
        const auto& B10 = B.variants[1];
        const auto& B01 = B.variants[2];
        const auto& B11 = B.variants[3];

        const auto& C = context.Observables()[2];
        ASSERT_EQ(C.variant_count, 4);
        const auto& C00 = C.variants[0];
        const auto& C10 = C.variants[1];
        const auto& C01 = C.variants[2];
        const auto& C11 = C.variants[3];

        // Make moment matrix
        const size_t mm_level = 1;
        const auto& mm = ims.MomentMatrix(mm_level);

        // Suggest extensions
        ExtensionSuggester suggester{context, symbols, factors};
        auto suggested_extensions = suggester(mm);
        const size_t extra_cols = suggested_extensions.size();
        ASSERT_GT(extra_cols, 0);

        // Make extended matrix
        const auto [em_id, em] = ims.ExtendedMatrices.create(ExtendedMatrixIndex{mm_level, suggested_extensions},
                                                             Multithreading::MultiThreadPolicy::Always);
        ASSERT_EQ(em_id, 1);
        EXPECT_EQ(em.OriginalDimension, mm.Dimension());
        EXPECT_EQ(em.Dimension(), mm.Dimension() + extra_cols);

        // Check MM is subset of EM
        const auto& mono_mm = dynamic_cast<const MonomialMatrix&>(mm);
        for (size_t col = 0; col < mm.Dimension(); ++col) {
            for (size_t row = 0; row < mm.Dimension(); ++row) {
                EXPECT_EQ(mono_mm.SymbolMatrix(col, row), em.SymbolMatrix(col, row))
                                    << "col = " << col << ", row = " << row;
            }
        }

    }
}