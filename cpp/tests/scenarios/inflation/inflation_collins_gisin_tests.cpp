/**
 * inflation_explicit_symbols_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_collins_gisin.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "matrix/operator_matrix/moment_matrix.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_CollinsGisin, W) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 3}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        oper_name_t a0 = context.Observables()[0].operator_offset;
        oper_name_t b0 = context.Observables()[1].operator_offset;
        oper_name_t c0 = context.Observables()[2].operator_offset;
        oper_name_t c1 = context.Observables()[2].operator_offset + 1;

        const auto* ePtr = symbols.where(OperatorSequence::Identity(context));

        const auto* a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        const auto* b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        const auto* c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        const auto* c1Ptr = symbols.where(OperatorSequence{{c1}, context});

        const auto* a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        const auto* a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        const auto* a0c1Ptr = symbols.where(OperatorSequence{{a0, c1}, context});
        const auto* b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        const auto* b0c1Ptr = symbols.where(OperatorSequence{{b0, c1}, context});

        const auto* a0b0c0Ptr = symbols.where(OperatorSequence{{a0, b0, c0}, context});
        const auto* a0b0c1Ptr = symbols.where(OperatorSequence{{a0, b0, c1}, context});

        ASSERT_NE(ePtr, nullptr);

        ASSERT_NE(a0Ptr, nullptr);
        ASSERT_NE(b0Ptr, nullptr);
        ASSERT_NE(c0Ptr, nullptr);
        ASSERT_NE(c1Ptr, nullptr);

        ASSERT_NE(a0b0Ptr, nullptr);
        ASSERT_NE(a0c0Ptr, nullptr);
        ASSERT_NE(a0c1Ptr, nullptr);
        ASSERT_NE(b0c0Ptr, nullptr);
        ASSERT_NE(b0c1Ptr, nullptr);

        ASSERT_NE(a0b0c0Ptr, nullptr);
        ASSERT_NE(a0b0c1Ptr, nullptr);


        // Make CG, and get object
        ims.RefreshCollinsGisin();
        const auto& collins_gisin = ims.CollinsGisin();
        ASSERT_EQ(collins_gisin.StorageType, TensorStorageType::Explicit);
        ASSERT_EQ(collins_gisin.DimensionCount, 8);
        ASSERT_EQ(collins_gisin.Dimensions, (std::vector<size_t>{2, 2, 2, 2, 2, 2, 3, 3})); // A0 A1 B0 B1 B2 B3 C0 C1
        ASSERT_EQ(collins_gisin.ElementCount, 576);
        EXPECT_FALSE(collins_gisin.HasAllSymbols()); // We don't have many 8-partite joint measurements.

        // I
        auto id_range = collins_gisin.measurement_to_range(std::vector<size_t>{});
        auto id_iter = id_range.begin();
        ASSERT_NE(id_iter, id_range.end());
        const auto& id_elem = *id_iter;
        EXPECT_EQ(id_elem.sequence, OperatorSequence::Identity(context));
        EXPECT_EQ(id_elem.symbol_id, ePtr->Id());
        ++id_iter;
        EXPECT_EQ(id_iter, id_range.end());

        // B0
        auto B0_range = collins_gisin.measurement_to_range(std::vector<size_t>{2});
        auto B0_iter = B0_range.begin();
        ASSERT_NE(B0_iter, B0_range.end());
        const auto& B0_elem = *B0_iter;
        EXPECT_EQ(B0_elem.sequence, OperatorSequence({b0}, context));
        EXPECT_EQ(B0_elem.symbol_id, b0Ptr->Id());
        ++B0_iter;
        EXPECT_EQ(B0_iter, B0_range.end());

        // A0B0
        auto A0B0_range = collins_gisin.measurement_to_range(std::vector<size_t>{0, 2});
        auto A0B0_iter = A0B0_range.begin();
        ASSERT_NE(A0B0_iter, A0B0_range.end());
        const auto& A0B0_elem = *A0B0_iter;
        EXPECT_EQ(A0B0_elem.sequence, OperatorSequence({a0, b0}, context));
        EXPECT_EQ(A0B0_elem.symbol_id, a0b0Ptr->Id());
        ++A0B0_iter;
        EXPECT_EQ(A0B0_iter, A0B0_range.end());

        // B0C0
        auto B0C0_range = collins_gisin.measurement_to_range(std::vector<size_t>{2, 6});
        auto B0C0_iter = B0C0_range.begin();
        ASSERT_NE(B0C0_iter, B0C0_range.end());
        const auto& B0C0_elem1 = *B0C0_iter;
        EXPECT_EQ(B0C0_elem1.sequence, OperatorSequence({b0, c0}, context));
        EXPECT_EQ(B0C0_elem1.symbol_id, b0c0Ptr->Id());
        ++B0C0_iter;
        ASSERT_NE(B0C0_iter, B0C0_range.end());
        const auto& B0C0_elem2 = *B0C0_iter;
        EXPECT_EQ(B0C0_elem2.sequence, OperatorSequence({b0, c1}, context));
        EXPECT_EQ(B0C0_elem2.symbol_id, b0c1Ptr->Id());
        ++B0C0_iter;
        EXPECT_EQ(B0C0_iter, B0C0_range.end());

        // A0B0C0
        auto ABC_range = collins_gisin.measurement_to_range(std::vector<size_t>{0, 2, 6});
        auto ABC_iter = ABC_range.begin();
        ASSERT_NE(ABC_iter, ABC_range.end());
        const auto& ABC_elem1 = *ABC_iter;
        EXPECT_EQ(ABC_elem1.sequence, OperatorSequence({a0, b0, c0}, context));
        EXPECT_EQ(ABC_elem1.symbol_id, a0b0c0Ptr->Id());
        ++ABC_iter;
        ASSERT_NE(ABC_iter, ABC_range.end());
        const auto& ABC_elem2 = *ABC_iter;
        EXPECT_EQ(ABC_elem2.sequence, OperatorSequence({a0, b0, c1}, context));
        EXPECT_EQ(ABC_elem2.symbol_id, a0b0c1Ptr->Id());
        ++ABC_iter;
        EXPECT_EQ(ABC_iter, ABC_range.end());
    }


//
//    TEST(Scenarios_Inflation_ExplicitSymbols, SingletonPair) {
//        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 0}, {{0, 1}}}, 2);
//        InflationMatrixSystem ims{std::move(icPtr)};
//        auto [id, momentMatrix] = ims.create_moment_matrix(2);
//        const auto& context = ims.InflationContext();
//        const auto& symbols = ims.Symbols();
//
//        // Operator IDs
//        ASSERT_EQ(context.Observables().size(), 3);
//        const auto& A = context.Observables()[0];
//        const auto& B = context.Observables()[1];
//        const auto& C = context.Observables()[2];
//
//        ASSERT_EQ(A.variants.size(), 2);
//        ASSERT_EQ(B.variants.size(), 2);
//        ASSERT_EQ(C.variants.size(), 1);
//        const auto& A0 = A.variants[0];
//        const auto& A1 = A.variants[1];
//        const auto& B0 = B.variants[0];
//        const auto& B1 = B.variants[1];
//        const auto& C0 = C.variants[0];
//
//        const auto a0 = A0.operator_offset;
//        const auto a1 = A1.operator_offset;
//        const auto b0 = B0.operator_offset;
//        const auto b1 = B1.operator_offset;
//        const auto c0 = C0.operator_offset;
//
//        std::set all_ids{a0,a1,b0,b1,c0};
//        ASSERT_EQ(all_ids.size(), 5);
//
//        const auto* ePtr = symbols.where(OperatorSequence::Identity(context));
//        ASSERT_NE(ePtr, nullptr);
//
//        const auto* a0Ptr = symbols.where(OperatorSequence{{a0}, context});
//        ASSERT_NE(a0Ptr, nullptr);
//        ASSERT_EQ(symbols.where(OperatorSequence{{a1}, context}), nullptr); // Symmetric to a0
//        const auto* b0Ptr = symbols.where(OperatorSequence{{b0}, context});
//        ASSERT_NE(b0Ptr, nullptr);
//        ASSERT_EQ(symbols.where(OperatorSequence{{b1}, context}), nullptr); // Symmetric to b0
//        const auto* c0Ptr = symbols.where(OperatorSequence{{c0}, context});
//        ASSERT_NE(c0Ptr, nullptr);
//
//        const auto* a0a1Ptr = symbols.where(OperatorSequence{{a0, a1}, context});
//        ASSERT_NE(a0a1Ptr, nullptr);
//        const auto* a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
//        ASSERT_NE(a0b0Ptr, nullptr);
//        const auto* a0b1Ptr = symbols.where(OperatorSequence{{a0, b1}, context});
//        ASSERT_NE(a0b1Ptr, nullptr);
//        const auto* a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
//        ASSERT_NE(a0c0Ptr, nullptr);
//        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b0}, context}), nullptr);// Symmetric to a0b1
//        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b1}, context}), nullptr); // Symmetric to a0b0
//        ASSERT_EQ(symbols.where(OperatorSequence{{a1, c0}, context}), nullptr); // Symmetric to a0c0
//        const auto* b0b1Ptr = symbols.where(OperatorSequence{{b0, b1}, context});
//        ASSERT_NE(b0b1Ptr, nullptr);
//        const auto* b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
//        ASSERT_NE(b0c0Ptr, nullptr);
//        ASSERT_EQ(symbols.where(OperatorSequence{{b1, c0}, context}), nullptr); // Symmetric to b0c0
//
//        const auto& explicit_symbols = ims.ExplicitSymbolTable();
//
//        // I
//        auto s_e = explicit_symbols.get(std::vector<OVIndex>{});
//        ASSERT_EQ(s_e.size(), 1);
//        EXPECT_EQ(s_e[0].symbol_id, ePtr->Id());
//
//        // A0
//        auto s_a0 = explicit_symbols.get(std::vector{OVIndex{0, 0}});
//        ASSERT_EQ(s_a0.size(), 1);
//        EXPECT_EQ(s_a0[0].symbol_id, a0Ptr->Id());
//
//        // A1
//        auto s_a1 = explicit_symbols.get(std::vector{OVIndex{0, 1}});
//        ASSERT_EQ(s_a1.size(), 1);
//        EXPECT_EQ(s_a1[0].symbol_id, a0Ptr->Id());
//
//        // B0
//        auto s_b0 = explicit_symbols.get(std::vector{OVIndex{1, 0}});
//        ASSERT_EQ(s_b0.size(), 1);
//        EXPECT_EQ(s_b0[0].symbol_id, b0Ptr->Id());
//
//        // B1
//        auto s_b1 = explicit_symbols.get(std::vector{OVIndex{1, 1}});
//        ASSERT_EQ(s_b1.size(), 1);
//        EXPECT_EQ(s_b1[0].symbol_id, b0Ptr->Id());
//
//        // C
//        auto s_c = explicit_symbols.get(std::vector{OVIndex{2, 0}});
//        ASSERT_EQ(s_c.size(), 1);
//        EXPECT_EQ(s_c[0].symbol_id, c0Ptr->Id());
//
//        // A0A1
//        auto s_a0a1 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{0, 1}});
//        ASSERT_EQ(s_a0a1.size(), 1);
//        EXPECT_EQ(s_a0a1[0].symbol_id, a0a1Ptr->Id());
//
//        // A0B0
//        auto s_a0b0 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 0}});
//        ASSERT_EQ(s_a0b0.size(), 1);
//        EXPECT_EQ(s_a0b0[0].symbol_id, a0b0Ptr->Id());
//
//        // A0B1
//        auto s_a0b1 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{1, 1}});
//        ASSERT_EQ(s_a0b1.size(), 1);
//        EXPECT_EQ(s_a0b1[0].symbol_id, a0b1Ptr->Id());
//
//        // A0C0
//        auto s_a0c0 = explicit_symbols.get(std::vector{OVIndex{0, 0}, OVIndex{2, 0}});
//        ASSERT_EQ(s_a0c0.size(), 1);
//        EXPECT_EQ(s_a0c0[0].symbol_id, a0c0Ptr->Id());
//
//        // A1B0
//        auto s_a1b0 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{1, 0}});
//        ASSERT_EQ(s_a1b0.size(), 1);
//        EXPECT_EQ(s_a1b0[0].symbol_id, a0b1Ptr->Id()); // symmetric
//
//        // A1B1
//        auto s_a1b1 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{1, 1}});
//        ASSERT_EQ(s_a1b1.size(), 1);
//        EXPECT_EQ(s_a1b1[0].symbol_id, a0b0Ptr->Id()); // symmetric
//
//        // A1C0
//        auto s_a1c0 = explicit_symbols.get(std::vector{OVIndex{0, 1}, OVIndex{2, 0}});
//        ASSERT_EQ(s_a1c0.size(), 1);
//        EXPECT_EQ(s_a1c0[0].symbol_id, a0c0Ptr->Id()); // symmetric
//
//        // B0B1
//        auto s_b0b1 = explicit_symbols.get(std::vector{OVIndex{1, 0}, OVIndex{1, 1}});
//        ASSERT_EQ(s_b0b1.size(), 1);
//        EXPECT_EQ(s_b0b1[0].symbol_id, b0b1Ptr->Id());
//
//        // B0C0
//        auto s_b0c0 = explicit_symbols.get(std::vector{OVIndex{1, 0}, OVIndex{2, 0}});
//        ASSERT_EQ(s_b0c0.size(), 1);
//        EXPECT_EQ(s_b0c0[0].symbol_id, b0c0Ptr->Id());
//
//        // B1C0
//        auto s_b1c0 = explicit_symbols.get(std::vector{OVIndex{1, 1}, OVIndex{2, 0}});
//        ASSERT_EQ(s_b1c0.size(), 1);
//        EXPECT_EQ(s_b1c0[0].symbol_id, b0c0Ptr->Id()); // symmetric
//    }
}