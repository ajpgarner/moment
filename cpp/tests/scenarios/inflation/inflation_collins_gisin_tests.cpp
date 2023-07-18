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
        ims.generate_dictionary(3);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        oper_name_t a0 = context.Observables()[0].operator_offset;
        oper_name_t b0 = context.Observables()[1].operator_offset;
        oper_name_t c0 = context.Observables()[2].operator_offset;
        oper_name_t c1 = context.Observables()[2].operator_offset + 1;

        const auto ePtr = symbols.where(OperatorSequence::Identity(context));

        const auto a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        const auto b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        const auto c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        const auto c1Ptr = symbols.where(OperatorSequence{{c1}, context});

        const auto a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        const auto a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        const auto a0c1Ptr = symbols.where(OperatorSequence{{a0, c1}, context});
        const auto b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        const auto b0c1Ptr = symbols.where(OperatorSequence{{b0, c1}, context});

        const auto a0b0c0Ptr = symbols.where(OperatorSequence{{a0, b0, c0}, context});
        const auto a0b0c1Ptr = symbols.where(OperatorSequence{{a0, b0, c1}, context});

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

    TEST(Scenarios_Inflation_CollinsGisin, OVOIndices_W) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 3}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        ims.generate_dictionary(3);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        oper_name_t a0 = context.Observables()[0].operator_offset;
        oper_name_t b0 = context.Observables()[1].operator_offset;
        oper_name_t c0 = context.Observables()[2].operator_offset;
        oper_name_t c1 = context.Observables()[2].operator_offset + 1;

        const auto ePtr = symbols.where(OperatorSequence::Identity(context));

        const auto a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        const auto b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        const auto c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        const auto c1Ptr = symbols.where(OperatorSequence{{c1}, context});

        const auto a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        const auto a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        const auto a0c1Ptr = symbols.where(OperatorSequence{{a0, c1}, context});
        const auto b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        const auto b0c1Ptr = symbols.where(OperatorSequence{{b0, c1}, context});

        const auto a0b0c0Ptr = symbols.where(OperatorSequence{{a0, b0, c0}, context});
        const auto a0b0c1Ptr = symbols.where(OperatorSequence{{a0, b0, c1}, context});

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
        const auto& collins_gisin = ims.InflationCollinsGisin();
        ASSERT_EQ(collins_gisin.StorageType, TensorStorageType::Explicit);
        ASSERT_EQ(collins_gisin.DimensionCount, 8);
        ASSERT_EQ(collins_gisin.Dimensions, (std::vector<size_t>{2, 2, 2, 2, 2, 2, 3, 3})); // A0 A1 B0 B1 B2 B3 C0 C1
        ASSERT_EQ(collins_gisin.ElementCount, 576);
        EXPECT_FALSE(collins_gisin.HasAllSymbols()); // We don't have many 8-partite joint measurements.

        // I
        const std::vector<OVIndex> id_index{};
        auto id_range = collins_gisin.measurement_to_range(id_index);
        auto id_iter = id_range.begin();
        ASSERT_NE(id_iter, id_range.end());
        const auto& id_elem = *id_iter;
        EXPECT_EQ(id_elem.sequence, OperatorSequence::Identity(context));
        EXPECT_EQ(id_elem.symbol_id, ePtr->Id());
        ++id_iter;
        EXPECT_EQ(id_iter, id_range.end());

        // B0
        const std::vector<OVIndex> B0_index{OVIndex{1, 0}};
        auto B0_range = collins_gisin.measurement_to_range(B0_index);
        auto B0_iter = B0_range.begin();
        ASSERT_NE(B0_iter, B0_range.end());
        const auto& B0_elem = *B0_iter;
        EXPECT_EQ(B0_elem.sequence, OperatorSequence({b0}, context));
        EXPECT_EQ(B0_elem.symbol_id, b0Ptr->Id());
        ++B0_iter;
        EXPECT_EQ(B0_iter, B0_range.end());

        // A0B0
        const std::vector<OVIndex> A0B0_index{OVIndex{0, 0}, OVIndex{1, 0}};
        auto A0B0_range = collins_gisin.measurement_to_range(A0B0_index);
        auto A0B0_iter = A0B0_range.begin();
        ASSERT_NE(A0B0_iter, A0B0_range.end());
        const auto& A0B0_elem = *A0B0_iter;
        EXPECT_EQ(A0B0_elem.sequence, OperatorSequence({a0, b0}, context));
        EXPECT_EQ(A0B0_elem.symbol_id, a0b0Ptr->Id());
        ++A0B0_iter;
        EXPECT_EQ(A0B0_iter, A0B0_range.end());

        // B0C0
        const std::vector<OVIndex> B0C0_index{OVIndex{1, 0}, OVIndex{2, 0}};
        auto B0C0_range = collins_gisin.measurement_to_range(B0C0_index);
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
        const std::vector<OVIndex> A0B0C0_index{OVIndex{0, 0}, OVIndex{1, 0}, OVIndex{2, 0}};
        auto ABC_range = collins_gisin.measurement_to_range(A0B0C0_index);
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

        // A0B0C0, fix C01.
        const std::vector<OVOIndex> fixed_C01{OVOIndex{2, 0, 1}};
        auto fixed_range = collins_gisin.measurement_to_range(A0B0_index, fixed_C01);
        auto fixed_iter = fixed_range.begin();
        ASSERT_NE(fixed_iter, fixed_range.end());
        const auto& fixed_elem1 = *fixed_iter;
        EXPECT_EQ(fixed_elem1.sequence, OperatorSequence({a0, b0, c1}, context));
        EXPECT_EQ(fixed_elem1.symbol_id, a0b0c1Ptr->Id());
        ++fixed_iter;
        EXPECT_EQ(fixed_iter, fixed_range.end());


    }


    namespace {
        void test_icg_mmt(const std::string &mmt_name, const InflationCollinsGisin &icg,
                          std::initializer_list<OVIndex> init_list,
                          const OperatorSequence& expected_os,
                          const Symbol& expected_sym) {
            const std::vector<OVIndex> indices(init_list.begin(), init_list.end());
            auto range = icg.measurement_to_range(indices);
            auto iter = range.begin();
            ASSERT_NE(iter, range.end()) << mmt_name;
            EXPECT_EQ(iter->sequence, expected_os) << mmt_name;
            EXPECT_EQ(iter->symbol_id, expected_sym.Id()) << mmt_name;
            EXPECT_EQ(iter->real_index, expected_sym.basis_key().first) << mmt_name;
            ++iter;
            EXPECT_EQ(iter, range.end()) << mmt_name;
        };
    }

    TEST(Scenarios_Inflation_CollinsGisin, SingletonPair) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2, 0}, {{0, 1}}}, 2)};
        auto [id, momentMatrix] = ims.MomentMatrix.create(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();
        ims.RefreshCollinsGisin();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        const auto& A = context.Observables()[0];
        const auto& B = context.Observables()[1];
        const auto& C = context.Observables()[2];

        ASSERT_EQ(A.variants.size(), 2);
        ASSERT_EQ(B.variants.size(), 2);
        ASSERT_EQ(C.variants.size(), 1);
        const auto& A0 = A.variants[0];
        const auto& A1 = A.variants[1];
        const auto& B0 = B.variants[0];
        const auto& B1 = B.variants[1];
        const auto& C0 = C.variants[0];

        const auto a0 = A0.operator_offset;
        const auto a1 = A1.operator_offset;
        const auto b0 = B0.operator_offset;
        const auto b1 = B1.operator_offset;
        const auto c0 = C0.operator_offset;

        std::set all_ids{a0,a1,b0,b1,c0};
        ASSERT_EQ(all_ids.size(), 5);

        const auto ePtr = symbols.where(OperatorSequence::Identity(context));
        ASSERT_NE(ePtr, nullptr);

        const auto a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        ASSERT_NE(a0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{a1}, context}).symbol, a0Ptr.symbol); // Symmetric to a0
        const auto b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        ASSERT_NE(b0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{b1}, context}).symbol, b0Ptr.symbol); // Symmetric to b0
        const auto c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        ASSERT_NE(c0Ptr, nullptr);

        const auto a0a1Ptr = symbols.where(OperatorSequence{{a0, a1}, context});
        ASSERT_NE(a0a1Ptr, nullptr);
        const auto a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        ASSERT_NE(a0b0Ptr, nullptr);
        const auto a0b1Ptr = symbols.where(OperatorSequence{{a0, b1}, context});
        ASSERT_NE(a0b1Ptr, nullptr);
        const auto a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        ASSERT_NE(a0c0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b0}, context}).symbol, a0b1Ptr.symbol);// Symmetric to a0b1
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, b1}, context}).symbol, a0b0Ptr.symbol); // Symmetric to a0b0
        ASSERT_EQ(symbols.where(OperatorSequence{{a1, c0}, context}).symbol, a0c0Ptr.symbol); // Symmetric to a0c0
        const auto b0b1Ptr = symbols.where(OperatorSequence{{b0, b1}, context});
        ASSERT_NE(b0b1Ptr, nullptr);
        const auto b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        ASSERT_NE(b0c0Ptr, nullptr);
        ASSERT_EQ(symbols.where(OperatorSequence{{b1, c0}, context}).symbol, b0c0Ptr.symbol); // Symmetric to b0c0

        const auto& icg = ims.InflationCollinsGisin();
        ASSERT_EQ(icg.StorageType, TensorStorageType::Explicit);
        const auto& data_elems = icg.Data();
        ASSERT_FALSE(data_elems.empty());

        // ID
        test_icg_mmt("I", icg, {}, OperatorSequence::Identity(context), *ePtr);

        // Single measurements
        test_icg_mmt("A0", icg, {OVIndex{0, 0}}, a0Ptr->sequence(), *a0Ptr);
        test_icg_mmt("A1", icg, {OVIndex{0, 1}}, OperatorSequence{{a1}, context}, *a0Ptr); // A0
        test_icg_mmt("B0", icg, {OVIndex{1, 0}}, b0Ptr->sequence(), *b0Ptr);
        test_icg_mmt("B1", icg, {OVIndex{1, 1}}, OperatorSequence{{b1}, context}, *b0Ptr); // B0
        test_icg_mmt("C", icg, {OVIndex{2, 0}}, c0Ptr->sequence(), *c0Ptr);

        // Joint measurements
        test_icg_mmt("A0A1", icg, {OVIndex{0, 0}, OVIndex{0, 1}}, a0a1Ptr->sequence(), *a0a1Ptr);
        test_icg_mmt("A0B0", icg, {OVIndex{0, 0}, OVIndex{1, 0}}, a0b0Ptr->sequence(), *a0b0Ptr);
        test_icg_mmt("A0B1", icg, {OVIndex{0, 0}, OVIndex{1, 1}}, a0b1Ptr->sequence(), *a0b1Ptr);
        test_icg_mmt("A0C0", icg, {OVIndex{0, 0}, OVIndex{2, 0}}, a0c0Ptr->sequence(), *a0c0Ptr);

        test_icg_mmt("A1B0", icg, {OVIndex{0, 1}, OVIndex{1, 0}}, OperatorSequence{{a1,b0}, context}, *a0b1Ptr); // A0B1
        test_icg_mmt("A1B1", icg, {OVIndex{0, 1}, OVIndex{1, 1}}, OperatorSequence{{a1,b1}, context}, *a0b0Ptr); // A0B0
        test_icg_mmt("A0C0", icg, {OVIndex{0, 1}, OVIndex{2, 0}}, OperatorSequence{{a1,c0}, context}, *a0c0Ptr); // A0C0

        test_icg_mmt("B0B1", icg, {OVIndex{1, 0}, OVIndex{1, 1}}, b0b1Ptr->sequence(), *b0b1Ptr);
        test_icg_mmt("B0C0", icg, {OVIndex{1, 0}, OVIndex{2, 0}}, b0c0Ptr->sequence(), *b0c0Ptr);
        test_icg_mmt("B1C0", icg, {OVIndex{1, 1}, OVIndex{2, 0}}, OperatorSequence{{b1,c0}, context}, *b0c0Ptr); // B0C0
    }
}