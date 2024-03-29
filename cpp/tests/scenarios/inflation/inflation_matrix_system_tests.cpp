/**
 * inflation_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "dictionary/operator_sequence_generator.h"
#include "matrix/operator_matrix/moment_matrix.h"

#include "symbolic/symbol_table.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/extension_suggester.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_MatrixSystem, FourOutcomeTriangle) {
        // Unit test to trigger bug encountered in matlab...

        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{4,      4,      4},
                                                                                   {{0, 1}, {1, 2}, {0, 2}}},
                                                                     2)};
        const auto &context = ims.InflationContext();
        const auto &symbols = ims.Symbols();

        ASSERT_EQ(context.Observables().size(), 3);
        const auto &A = context.Observables()[0];
        ASSERT_EQ(A.variant_count, 4);
        const auto &A00 = A.variants[0];
        const auto &A10 = A.variants[1];
        const auto &A01 = A.variants[2];
        const auto &A11 = A.variants[3];

        const auto &B = context.Observables()[1];
        ASSERT_EQ(B.variant_count, 4);
        const auto &B00 = B.variants[0];
        const auto &B10 = B.variants[1];
        const auto &B01 = B.variants[2];
        const auto &B11 = B.variants[3];

        const auto &C = context.Observables()[2];
        ASSERT_EQ(C.variant_count, 4);
        const auto &C00 = C.variants[0];
        const auto &C10 = C.variants[1];
        const auto &C01 = C.variants[2];
        const auto &C11 = C.variants[3];

        ASSERT_EQ(context.Sources().size(), 3);

        std::vector<oper_name_t> all_variants{A00.operator_offset, A10.operator_offset,
                                              A01.operator_offset, A11.operator_offset,
                                              B00.operator_offset, B10.operator_offset,
                                              B01.operator_offset, B11.operator_offset,
                                              C00.operator_offset, C10.operator_offset,
                                              C01.operator_offset, C11.operator_offset};
        ASSERT_TRUE(std::is_sorted(all_variants.cbegin(), all_variants.cend()));
        ASSERT_TRUE(std::all_of(all_variants.cbegin(), all_variants.cend(), [](const oper_name_t op_num) {
            return (op_num % 3) == 0;
        }));

        // Construct and verify sequences A00 C00 C11
        const OperatorSequence seq_A00C00C11{{A00.operator_offset, C00.operator_offset, C11.operator_offset}, context};
        ASSERT_EQ(seq_A00C00C11.size(), 3);
        ASSERT_EQ(seq_A00C00C11[0], A00.operator_offset);
        ASSERT_EQ(seq_A00C00C11[1], C00.operator_offset);
        ASSERT_EQ(seq_A00C00C11[2], C11.operator_offset);
        ASSERT_EQ(OperatorSequence(seq_A00C00C11), seq_A00C00C11);


        // Construct and verify sequences A10 C00 C11
        const OperatorSequence seq_A10C00C11{{A10.operator_offset, C00.operator_offset, C11.operator_offset}, context};
        ASSERT_EQ(seq_A10C00C11.size(), 3);
        ASSERT_EQ(seq_A10C00C11[0], A10.operator_offset);
        ASSERT_EQ(seq_A10C00C11[1], C00.operator_offset);
        ASSERT_EQ(seq_A10C00C11[2], C11.operator_offset);
        ASSERT_EQ(OperatorSequence(seq_A10C00C11), seq_A10C00C11);

        // Construct and verify sequences A00 C10 C01
        const OperatorSequence seq_A00C10C01{{A00.operator_offset, C10.operator_offset, C01.operator_offset}, context};
        ASSERT_EQ(seq_A00C10C01.size(), 3);
        ASSERT_EQ(seq_A00C10C01[0], A00.operator_offset);
        ASSERT_EQ(seq_A00C10C01[1], C10.operator_offset);
        ASSERT_EQ(seq_A00C10C01[2], C01.operator_offset);
        ASSERT_EQ(OperatorSequence(seq_A00C10C01), seq_A00C10C01);

        // Construct and verify sequences A01 C00 C11
        const OperatorSequence seq_A01C00C11{{A01.operator_offset, C00.operator_offset, C11.operator_offset}, context};
        ASSERT_EQ(seq_A01C00C11.size(), 3);
        ASSERT_EQ(seq_A01C00C11[0], A01.operator_offset);
        ASSERT_EQ(seq_A01C00C11[1], C00.operator_offset);
        ASSERT_EQ(seq_A01C00C11[2], C11.operator_offset);
        ASSERT_EQ(OperatorSequence(seq_A01C00C11), seq_A01C00C11);

        // A00 C00 C11 cannot be simplified
        EXPECT_FALSE(context.can_be_simplified_as_moment(seq_A00C00C11));
        EXPECT_EQ(context.simplify_as_moment(OperatorSequence(seq_A00C00C11)), seq_A00C00C11);

        // alias: A10 C00 C11 -> A00 C00 C11 on unshared x index of A.
        EXPECT_TRUE(context.can_be_simplified_as_moment(seq_A10C00C11));
        EXPECT_EQ(context.simplify_as_moment(OperatorSequence(seq_A10C00C11)), seq_A00C00C11);

        // alias: A00 C10 C01 -> A00 C00 C11
        EXPECT_TRUE(context.can_be_simplified_as_moment(seq_A00C10C01));
        EXPECT_EQ(context.simplify_as_moment(OperatorSequence(seq_A00C10C01)), seq_A00C00C11);

        // alias: A01 C10 C01 -> A00 C00 C11
        EXPECT_TRUE(context.can_be_simplified_as_moment(seq_A01C00C11));
        EXPECT_EQ(context.simplify_as_moment(OperatorSequence(seq_A01C00C11)), seq_A00C00C11);

        //A10.operator_offset -> 6
        //C00.operator_offset -> 24
        //C11.operator_offset -> 33

        ims.generate_dictionary(3);
        auto symbol_A00C00C11 = symbols.where(seq_A00C00C11);
        EXPECT_TRUE(symbol_A00C00C11.found());
        EXPECT_FALSE(symbol_A00C00C11.is_aliased);

        auto symbol_A10C00C11 = symbols.where(seq_A10C00C11);
        EXPECT_TRUE(symbol_A10C00C11.found());
        EXPECT_TRUE(symbol_A10C00C11.is_aliased);

        auto symbol_A00C10C01 = symbols.where(seq_A00C10C01);
        EXPECT_TRUE(symbol_A00C10C01.found());
        EXPECT_TRUE(symbol_A00C10C01.is_aliased);

    }


    TEST(Scenarios_Inflation_MatrixSystem, LevelThreeRefreshProbability) {
        // 'bad allocation' bug found via matlab
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{4, 4, 4},
                                                                                   {{0, 1}, {1, 2}, {0, 2}}},
                                                                     3)};
        const auto &context = ims.InflationContext();
        const auto &symbols = ims.Symbols();

        const auto& mm = ims.MomentMatrix(1);
        ims.RefreshProbabilityTensor();
    }

    TEST(Scenarios_Inflation_MatrixSystem, Point_Five) {
        // Unit test to trigger another bug encountered in matlab...

        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}}}, 1)};
        const auto &context = ims.InflationContext();

        const auto& dict3 = context.operator_sequence_generator(3); // Longest entry: a0 b0 c0
        const auto& dict4 = context.operator_sequence_generator(4); // Longest entry is still a0 b0 c0!
        EXPECT_EQ(dict3.size(), dict4.size());
    }
}