/**
 * factor_table_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "symbolic/symbol_table.h"

#include "scenarios/inflation/canonical_observables.h"
#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_FactorTable, Empty) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2, 2},
                                                                   {{0, 1}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();

        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 2);

        // Zero
        const auto &factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);

        // ID
        const auto &factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);

        // Do we find 0 and 1 by factor?
        auto find_zero = factors.find_index_by_factors({0});
        ASSERT_TRUE(find_zero.has_value());
        EXPECT_EQ(find_zero.value(), factors_0.id);

        auto find_one = factors.find_index_by_factors({1});
        ASSERT_TRUE(find_one.has_value());
        EXPECT_EQ(find_one.value(), factors_I.id);


    }


    TEST(Scenarios_Inflation_FactorTable, UnlinkedPair) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2, 2},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();

        const auto &mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, AB
        ASSERT_EQ(ims.Symbols().size(), 5);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 5);

        // Zero
        const auto &factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);
        auto find_zero = factors.find_index_by_factors({0});
        ASSERT_TRUE(find_zero.has_value());
        EXPECT_EQ(find_zero.value(), factors_0.id);

        // ID
        const auto &factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);
        auto find_I = factors.find_index_by_factors({1});
        ASSERT_TRUE(find_I.has_value());
        EXPECT_EQ(find_I.value(), factors_I.id);

        // A
        const auto &factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);
        auto find_A = factors.find_index_by_factors({2});
        ASSERT_TRUE(find_A.has_value());
        EXPECT_EQ(find_A.value(), factors_A.id);

        // B
        const auto &factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);
        auto find_B = factors.find_index_by_factors({3});
        ASSERT_TRUE(find_B.has_value());
        EXPECT_EQ(find_B.value(), factors_B.id);

        // AB -> A, B
        const auto &factors_AB = factors[4];
        EXPECT_EQ(factors_AB.id, 4);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.raw.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.canonical.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AB.canonical.symbols[1], 3);
        auto find_AB = factors.find_index_by_factors({2, 3});
        ASSERT_TRUE(find_AB.has_value());
        EXPECT_EQ(find_AB.value(), factors_AB.id);


    }

    TEST(Scenarios_Inflation_FactorTable, UnlinkedCVPair) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();
        const auto &symbols = ims.Symbols();

        const auto &mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, A^2, AB, B^2
        ASSERT_EQ(symbols.size(), 7);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 7);

        // Zero
        const auto &factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_0.canonical.sequences[0], OperatorSequence::Zero(context));
        ASSERT_EQ(factors_0.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_0.canonical.symbols[0], 0);
        auto find_zero = factors.find_index_by_factors({0});
        ASSERT_TRUE(find_zero.has_value());
        EXPECT_EQ(find_zero.value(), factors_0.id);


        // ID
        const auto &factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);
        auto find_I = factors.find_index_by_factors({1});
        ASSERT_TRUE(find_I.has_value());
        EXPECT_EQ(find_I.value(), factors_I.id);

        // A
        const auto &factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);
        auto find_A = factors.find_index_by_factors({2});
        ASSERT_TRUE(find_A.has_value());
        EXPECT_EQ(find_A.value(), factors_A.id);

        // B
        const auto &factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);
        auto find_B = factors.find_index_by_factors({3});
        ASSERT_TRUE(find_B.has_value());
        EXPECT_EQ(find_B.value(), factors_B.id);

        // AA
        const auto &factors_AA = factors[4];
        EXPECT_EQ(factors_AA.id, 4);
        ASSERT_EQ(factors_AA.raw.sequences.size(), 1) << symbols << ims.CanonicalObservables();
        EXPECT_EQ(factors_AA.raw.sequences[0], OperatorSequence({0, 0}, context));
        ASSERT_EQ(factors_AA.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_AA.canonical.sequences[0], OperatorSequence({0, 0}, context));
        ASSERT_EQ(factors_AA.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_AA.canonical.symbols[0], 4);
        auto find_AA = factors.find_index_by_factors({4});
        ASSERT_TRUE(find_AA.has_value());
        EXPECT_EQ(find_AA.value(), factors_AA.id);


        // AB -> A, B
        const auto &factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.raw.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AB.canonical.sequences[1], OperatorSequence({1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AB.canonical.symbols[1], 3);
        auto find_AB = factors.find_index_by_factors({2, 3});
        ASSERT_TRUE(find_AB.has_value());
        EXPECT_EQ(find_AB.value(), factors_AB.id);

        // BB
        const auto &factors_BB = factors[6];
        EXPECT_EQ(factors_BB.id, 6);
        ASSERT_EQ(factors_BB.raw.sequences.size(), 1);
        EXPECT_EQ(factors_BB.raw.sequences[0], OperatorSequence({1, 1}, context));
        ASSERT_EQ(factors_BB.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_BB.canonical.sequences[0], OperatorSequence({1, 1}, context));
        ASSERT_EQ(factors_BB.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_BB.canonical.symbols[0], 6);
        auto find_BB = factors.find_index_by_factors({6});
        ASSERT_TRUE(find_BB.has_value());
        EXPECT_EQ(find_BB.value(), factors_BB.id);
    }

    TEST(Scenarios_Inflation_FactorTable, W) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{2,      2, 2},
                                                                   {{0, 1}, {1, 2}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();

        const auto &mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, C, AB, AC, BC
        ASSERT_EQ(ims.Symbols().size(), 8);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 8);

        // Zero
        const auto &factors_0 = factors[0];
        EXPECT_EQ(factors_0.id, 0);
        ASSERT_EQ(factors_0.raw.sequences.size(), 1);
        EXPECT_EQ(factors_0.raw.sequences[0], OperatorSequence::Zero(context));
        auto find_zero = factors.find_index_by_factors({0});
        ASSERT_TRUE(find_zero.has_value());
        EXPECT_EQ(find_zero.value(), factors_0.id);

        // ID
        const auto &factors_I = factors[1];
        EXPECT_EQ(factors_I.id, 1);
        ASSERT_EQ(factors_I.raw.sequences.size(), 1);
        EXPECT_EQ(factors_I.raw.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_I.canonical.sequences[0], OperatorSequence::Identity(context));
        ASSERT_EQ(factors_I.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_I.canonical.symbols[0], 1);
        auto find_I = factors.find_index_by_factors({1});
        ASSERT_TRUE(find_I.has_value());
        EXPECT_EQ(find_I.value(), factors_I.id);

        // A
        const auto &factors_A = factors[2];
        EXPECT_EQ(factors_A.id, 2);
        ASSERT_EQ(factors_A.raw.sequences.size(), 1);
        EXPECT_EQ(factors_A.raw.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_A.canonical.sequences[0], OperatorSequence({0}, context));
        ASSERT_EQ(factors_A.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_A.canonical.symbols[0], 2);
        auto find_A = factors.find_index_by_factors({2});
        ASSERT_TRUE(find_A.has_value());
        EXPECT_EQ(find_A.value(), factors_A.id);

        // B
        const auto &factors_B = factors[3];
        EXPECT_EQ(factors_B.id, 3);
        ASSERT_EQ(factors_B.raw.sequences.size(), 1);
        EXPECT_EQ(factors_B.raw.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_B.canonical.sequences[0], OperatorSequence({1}, context));
        ASSERT_EQ(factors_B.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_B.canonical.symbols[0], 3);
        auto find_B = factors.find_index_by_factors({3});
        ASSERT_TRUE(find_B.has_value());
        EXPECT_EQ(find_B.value(), factors_B.id);

        // C
        const auto &factors_C = factors[4];
        EXPECT_EQ(factors_C.id, 4);
        ASSERT_EQ(factors_C.raw.sequences.size(), 1);
        EXPECT_EQ(factors_C.raw.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_C.canonical.sequences[0], OperatorSequence({2}, context));
        ASSERT_EQ(factors_C.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_C.canonical.symbols[0], 4);
        auto find_C = factors.find_index_by_factors({4});
        ASSERT_TRUE(find_C.has_value());
        EXPECT_EQ(find_C.value(), factors_C.id);

        // AB -> AB
        const auto &factors_AB = factors[5];
        EXPECT_EQ(factors_AB.id, 5);
        ASSERT_EQ(factors_AB.raw.sequences.size(), 1);
        EXPECT_EQ(factors_AB.raw.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_AB.canonical.sequences[0], OperatorSequence({0, 1}, context));
        ASSERT_EQ(factors_AB.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_AB.canonical.symbols[0], 5);
        auto find_AB = factors.find_index_by_factors({5});
        ASSERT_TRUE(find_AB.has_value());
        EXPECT_EQ(find_AB.value(), factors_AB.id);

        // AC -> A, C
        const auto &factors_AC = factors[6];
        EXPECT_EQ(factors_AC.id, 6);
        ASSERT_EQ(factors_AC.raw.sequences.size(), 2);
        EXPECT_EQ(factors_AC.raw.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.raw.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.canonical.sequences.size(), 2);
        EXPECT_EQ(factors_AC.canonical.sequences[0], OperatorSequence({0}, context));
        EXPECT_EQ(factors_AC.canonical.sequences[1], OperatorSequence({2}, context));
        ASSERT_EQ(factors_AC.canonical.symbols.size(), 2);
        EXPECT_EQ(factors_AC.canonical.symbols[0], 2);
        EXPECT_EQ(factors_AC.canonical.symbols[1], 4);
        auto find_AC = factors.find_index_by_factors({2, 4});
        ASSERT_TRUE(find_AC.has_value());
        EXPECT_EQ(find_AC.value(), factors_AC.id);

        // BC -> BC
        const auto &factors_BC = factors[7];
        EXPECT_EQ(factors_BC.id, 7);
        ASSERT_EQ(factors_BC.raw.sequences.size(), 1);
        EXPECT_EQ(factors_BC.raw.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.canonical.sequences.size(), 1);
        EXPECT_EQ(factors_BC.canonical.sequences[0], OperatorSequence({1, 2}, context));
        ASSERT_EQ(factors_BC.canonical.symbols.size(), 1);
        EXPECT_EQ(factors_BC.canonical.symbols[0], 7);
        auto find_BC = factors.find_index_by_factors({7});
        ASSERT_TRUE(find_BC.has_value());
        EXPECT_EQ(find_BC.value(), factors_BC.id);
    }

    TEST(Scenarios_Inflation_FactorTable, RegisterFactorsOnDictionaryGeneration) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0, 0},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();
        const auto &symbols = ims.Symbols();

        ims.generate_dictionary(2);
        EXPECT_EQ(factors.size(), symbols.size());
    }

    TEST(Scenarios_Inflation_FactorTable, Combine_Regular) {
        std::vector<symbol_name_t> left{2};
        std::vector<symbol_name_t> right{3};

        auto lr = FactorTable::combine_symbolic_factors(left, right);
        ASSERT_EQ(lr.size(), 2);
        EXPECT_EQ(lr[0], 2);
        EXPECT_EQ(lr[1], 3);

        auto rl = FactorTable::combine_symbolic_factors(right, left);
        ASSERT_EQ(rl.size(), 2);
        EXPECT_EQ(rl[0], 2);
        EXPECT_EQ(rl[1], 3);
    }

    TEST(Scenarios_Inflation_FactorTable, Combine_WithID) {
        std::vector<symbol_name_t> left{2};
        std::vector<symbol_name_t> right{1};

        auto lr = FactorTable::combine_symbolic_factors(left, right);
        ASSERT_EQ(lr.size(), 1);
        EXPECT_EQ(lr[0], 2);

        auto rl = FactorTable::combine_symbolic_factors(right, left);
        ASSERT_EQ(rl.size(), 1);
        EXPECT_EQ(rl[0], 2);
    }

    TEST(Scenarios_Inflation_FactorTable, Combine_MultiID) {
        std::vector<symbol_name_t> left{1};
        std::vector<symbol_name_t> right{1};

        auto lr = FactorTable::combine_symbolic_factors(left, right);
        ASSERT_EQ(lr.size(), 1);
        EXPECT_EQ(lr[0], 1);

        auto rl = FactorTable::combine_symbolic_factors(right, left);
        ASSERT_EQ(rl.size(), 1);
        EXPECT_EQ(rl[0], 1);
    }

    TEST(Scenarios_Inflation_FactorTable, TryProduct_PairUnlinkedCV) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();
        const auto &symbols = ims.Symbols();

        const auto &mm = ims.create_moment_matrix(1); // Symbols: 0, I, A, B, A^2, AB, B^2
        ASSERT_EQ(symbols.size(), 7);
        EXPECT_FALSE(factors.empty());
        ASSERT_EQ(factors.size(), 7);

        auto abIdx = factors.find_index_by_factors({2, 3}); // A, B -> <AB> = <A><B>
        ASSERT_TRUE(abIdx.has_value());

        EXPECT_EQ(factors.try_multiply(1, 0), 0);
        EXPECT_EQ(factors.try_multiply(2, 0), 0);
        EXPECT_EQ(factors.try_multiply(0, 1), 0);
        EXPECT_EQ(factors.try_multiply(0, 2), 0);
        EXPECT_EQ(factors.try_multiply(1, 1), 1);
        EXPECT_EQ(factors.try_multiply(1, 2), 2);
        EXPECT_EQ(factors.try_multiply(2, 1), 2);
        EXPECT_EQ(factors.try_multiply(2, 3), abIdx.value());
        EXPECT_EQ(factors.try_multiply(3, 2), abIdx.value());
        EXPECT_THROW([[maybe_unused]] auto err = factors.try_multiply(2, 2), Moment::Inflation::errors::unknown_symbol);
    }

    TEST(Scenarios_Inflation_FactorTable, TryProduct_UnlinkedCVTriplet) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0, 0},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();
        const auto &symbols = ims.Symbols();

        ims.generate_dictionary(3);


        auto maybe_AB = factors.find_index_by_factors({2, 3}); // A, B -> <AB> = <A><B>
        auto maybe_BC = factors.find_index_by_factors({3, 4}); // B, C -> <BB> = <A><C>
        auto maybe_AC = factors.find_index_by_factors({2, 4}); // A, C -> <AC> = <B><C>
        auto maybe_ABC = factors.find_index_by_factors({2, 3, 4}); // A, B, C-> <ABC> = <A><B>
        ASSERT_TRUE(maybe_AB.has_value());
        ASSERT_TRUE(maybe_BC.has_value());
        ASSERT_TRUE(maybe_AC.has_value());
        ASSERT_TRUE(maybe_ABC.has_value());

        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{2, 1, 1}), 2);
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{2, 1, 0}), 0);

        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{2, 3}), maybe_AB.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{1, 3, 2}), maybe_AB.value());

        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{3, 4}), maybe_BC.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{2, 4}), maybe_AC.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{2, 3, 4}), maybe_ABC.value());

        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{4, 3, 2}), maybe_ABC.value());

        // <AB><C> -> <ABC>, <AC><B> -> <ABC> and <BC><A> -> <ABC>
        EXPECT_EQ(factors.try_multiply(maybe_AB.value(), 4), maybe_ABC.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{maybe_AB.value(), 4}), maybe_ABC.value());
        EXPECT_EQ(factors.try_multiply(maybe_AC.value(), 3), maybe_ABC.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{maybe_AC.value(), 3}), maybe_ABC.value());
        EXPECT_EQ(factors.try_multiply(maybe_BC.value(), 2), maybe_ABC.value());
        EXPECT_EQ(factors.try_multiply(std::vector<symbol_name_t>{maybe_BC.value(), 2}), maybe_ABC.value());

        EXPECT_THROW([[maybe_unused]] auto err = factors.try_multiply(std::vector<symbol_name_t>{1, 2, 2, 3, 3}),
                     Moment::Inflation::errors::unknown_symbol);
    }

    TEST(Scenarios_Inflation_FactorTable, TryProduct_UnlinkedCVTriplet_SymbolCombo) {
        std::unique_ptr<InflationContext> icPtr
                = std::make_unique<InflationContext>(CausalNetwork{{0, 0, 0},
                                                                   {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &factors = ims.Factors();
        const auto &symbols = ims.Symbols();
        SymbolComboFactory factory{symbols};

        ims.generate_dictionary(3);
        auto maybe_AB = factors.find_index_by_factors({2, 3}); // A, B -> <AB> = <A><B>
        auto maybe_BC = factors.find_index_by_factors({3, 4}); // B, C -> <BB> = <A><C>
        auto maybe_AC = factors.find_index_by_factors({2, 4}); // A, C -> <AC> = <B><C>
        auto maybe_ABC = factors.find_index_by_factors({2, 3, 4}); // A, B, C-> <ABC> = <A><B>
        ASSERT_TRUE(maybe_AB.has_value());
        ASSERT_TRUE(maybe_BC.has_value());
        ASSERT_TRUE(maybe_AC.has_value());
        ASSERT_TRUE(maybe_ABC.has_value());

        const auto one_plus_A = factory({Monomial{1, 1.0}, Monomial{2, 1.0}});
        const auto one_plus_B = factory({Monomial{1, 1.0}, Monomial{3, 1.0}});
        const auto one_plus_C = factory({Monomial{1, 1.0}, Monomial{4, 1.0}});
        const auto expected_IplusA_IplusB = factory({Monomial{1, 1.0}, Monomial{2, 1.0},
                                                     Monomial{3, 1.0}, Monomial{maybe_AB.value(), 1.0}});
        const auto expected_IA_IB_IC = factory({Monomial{1, 1.0},
                                                Monomial{2, 1.0},
                                                Monomial{3, 1.0},
                                                Monomial{4, 1.0},
                                                Monomial{maybe_AB.value(), 1.0},
                                                Monomial{maybe_BC.value(), 1.0},
                                                Monomial{maybe_AC.value(), 1.0},
                                                Monomial{maybe_ABC.value(), 1.0}});
        EXPECT_EQ(factors.try_multiply(factory, one_plus_A, one_plus_B), expected_IplusA_IplusB);
        EXPECT_EQ(factors.try_multiply(factory, expected_IplusA_IplusB, one_plus_C), expected_IA_IB_IC);

        const auto expected_one_plus_A_times05 = factory({Monomial{1, 0.5}, Monomial{2, 0.5}});
        EXPECT_EQ(factors.try_multiply(factory, one_plus_A, Polynomial::Scalar(0.5)),
                  expected_one_plus_A_times05 );


    }
}