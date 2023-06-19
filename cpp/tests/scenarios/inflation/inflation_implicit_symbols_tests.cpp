/**
 * inflation_implicit_symbols_tests.cpp.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/implicit_symbols.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_implicit_symbols.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../implicit_symbol_test_helpers.h"

#include <set>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_ImplicitSymbols, Empty) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        EXPECT_EQ(implSym.MaxSequenceLength, 0);
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 1);

        const auto& one = implSym.Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        Polynomial oneCombo{Monomial{1, 1.0}};
        EXPECT_EQ(one.expression, oneCombo);

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);
        EXPECT_EQ(&getOne[0], &one);
    }

    TEST(Scenarios_Inflation_ImplicitSymbols, Singleton) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1); // should be [[1 A]; [A A]]
        const auto& implSym = ims.ImplicitSymbolTable();

        EXPECT_EQ(implSym.MaxSequenceLength, 1);
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 3); // e, a0, a1

        const auto& one = implSym.Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        Polynomial oneCombo{Monomial{1, 1.0}};
        EXPECT_EQ(one.expression, oneCombo);

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);
        EXPECT_EQ(&getOne[0], &one);

        const auto& getA = implSym.get(std::vector<OVIndex>{{0, 0}});
        test2Mmt(getA, 1, 2);
    }

    TEST(Scenarios_Inflation_ImplicitSymbols, Singleton_Cloned) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        ASSERT_EQ(implSym.MaxSequenceLength, 2); // now we have A0A1 too
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 7); // e, a0 [2], a0a1 [4]

        const auto& getOne = implSym.get(std::vector<OVIndex>{});
        ASSERT_EQ(getOne.size(), 1);
        EXPECT_EQ(getOne[0].symbol_id, 1);

        const auto& getA = implSym.get(std::vector<OVIndex>{{0, 0}});
        test2Mmt(getA, 1, 2, "A0");

        const auto& getAprime = implSym.get(std::vector<OVIndex>{{0, 1}});
        test2Mmt(getAprime, 1, 2, "A1");

        const auto& getAAprime = implSym.get(std::vector<OVIndex>{{0, 0}, {0, 1}});
        test22JoinMmt(getAAprime, 1, 2, 2, 3, "A0A1");
    }

    TEST(Scenarios_Inflation_ImplicitSymbols, Pair_OneCV) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 0}, {{0, 1}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

    }

    TEST(Scenarios_Inflation_ImplicitSymbols, Block) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        ASSERT_EQ(implSym.MaxSequenceLength, 2); // now we have A0A1 too
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 7); // e, a0 [2], a0a1 [4]

        auto eBlock = implSym.Block(0);
        ASSERT_EQ(eBlock.size(), 1);
        EXPECT_EQ(eBlock[0].symbol_id, 1);

        auto a0Block = implSym.Block(1);
        ASSERT_EQ(a0Block.size(), 2);
        EXPECT_EQ(a0Block[0].symbol_id, 2);
        EXPECT_EQ(a0Block[1].symbol_id, -1);

    }

    TEST(Scenarios_Inflation_ImplicitSymbols, ITE_22) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {{0, 1}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        const oper_name_t op_A = context.Observables()[0].operator_offset;
        const oper_name_t op_B = context.Observables()[1].operator_offset;

        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        auto id_ePtr = symbols.where(OperatorSequence::Identity(context));
        ASSERT_NE(id_ePtr, nullptr);
        auto id_e  = id_ePtr->Id();

        auto id_APtr = symbols.where(OperatorSequence{{op_A}, context});
        ASSERT_NE(id_APtr, nullptr);
        auto id_A  = id_APtr->Id();

        auto id_BPtr = symbols.where(OperatorSequence{{op_B}, context});
        ASSERT_NE(id_BPtr, nullptr);
        auto id_B  = id_BPtr->Id();

        auto id_ABPtr = symbols.where(OperatorSequence{{op_A, op_B}, context});
        ASSERT_NE(id_ABPtr, nullptr);
        auto id_AB  = id_ABPtr->Id();

        std::vector<OVIndex> mmts{{0, 0}, {1, 0}};
        std::vector<double> distribution{0.1, 0.2, 0.3, 0.4};

        auto explicit_form = implSym.implicit_to_explicit(mmts, distribution);
        ASSERT_EQ(explicit_form.size(), 4);

        auto find_e = explicit_form.find(id_e);
        ASSERT_NE(find_e, explicit_form.end());
        auto find_A = explicit_form.find(id_A);
        ASSERT_NE(find_A, explicit_form.end());
        auto find_B = explicit_form.find(id_B);
        ASSERT_NE(find_B, explicit_form.end());
        auto find_AB = explicit_form.find(id_AB);
        ASSERT_NE(find_AB, explicit_form.end());

        EXPECT_FLOAT_EQ(find_e->second, 1.0);
        EXPECT_FLOAT_EQ(find_A->second, 0.3);
        EXPECT_FLOAT_EQ(find_B->second, 0.4);
        EXPECT_FLOAT_EQ(find_AB->second, 0.1);
    }

    TEST(Scenarios_Inflation_ImplicitSymbols, ITE_32) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{3, 2}, {{0, 1}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        const oper_name_t op_A0 = context.Observables()[0].operator_offset;
        const oper_name_t op_A1 = context.Observables()[0].operator_offset + 1;
        const oper_name_t op_B = context.Observables()[1].operator_offset;

        auto [id, momentMatrix] = ims.create_moment_matrix(1);
        const auto& implSym = ims.ImplicitSymbolTable();

        auto id_ePtr = symbols.where(OperatorSequence::Identity(context));
        ASSERT_NE(id_ePtr, nullptr);
        auto id_e  = id_ePtr->Id();

        auto id_A0Ptr = symbols.where(OperatorSequence{{op_A0}, context});
        ASSERT_NE(id_A0Ptr, nullptr);
        auto id_A0  = id_A0Ptr->Id();

        auto id_A1Ptr = symbols.where(OperatorSequence{{op_A1}, context});
        ASSERT_NE(id_A1Ptr, nullptr);
        auto id_A1  = id_A1Ptr->Id();

        auto id_BPtr = symbols.where(OperatorSequence{{op_B}, context});
        ASSERT_NE(id_BPtr, nullptr);
        auto id_B  = id_BPtr->Id();

        auto id_A0BPtr = symbols.where(OperatorSequence{{op_A0, op_B}, context});
        ASSERT_NE(id_A0BPtr, nullptr);
        auto id_A0B  = id_A0BPtr->Id();

        auto id_A1BPtr = symbols.where(OperatorSequence{{op_A1, op_B}, context});
        ASSERT_NE(id_A1BPtr, nullptr);
        auto id_A1B = id_A1BPtr->Id();

        std::set all_ids{id_e, id_A0, id_A1, id_B, id_A0B, id_A1B};
        ASSERT_EQ(all_ids.size(), 6);

        std::vector<OVIndex> mmts{{0, 0}, {1, 0}};
        std::vector<double> distribution{0.05, 0.05, 0.1, 0.2, 0.25, 0.35};

        auto explicit_form = implSym.implicit_to_explicit(mmts, distribution);
        ASSERT_EQ(explicit_form.size(), 6);

        auto find_e = explicit_form.find(id_e);
        ASSERT_NE(find_e, explicit_form.end());
        auto find_A0 = explicit_form.find(id_A0);
        ASSERT_NE(find_A0, explicit_form.end());
        auto find_A1 = explicit_form.find(id_A1);
        ASSERT_NE(find_A1, explicit_form.end());
        auto find_B = explicit_form.find(id_B);
        ASSERT_NE(find_B, explicit_form.end());
        auto find_A0B = explicit_form.find(id_A0B);
        ASSERT_NE(find_A0B, explicit_form.end());
        auto find_A1B = explicit_form.find(id_A1B);
        ASSERT_NE(find_A1B, explicit_form.end());

        EXPECT_FLOAT_EQ(find_e->second,   1.0);
        EXPECT_FLOAT_EQ(find_A0->second,  0.1);
        EXPECT_FLOAT_EQ(find_A1->second,  0.3);
        EXPECT_FLOAT_EQ(find_B->second,   0.4);
        EXPECT_FLOAT_EQ(find_A0B->second, 0.05);
        EXPECT_FLOAT_EQ(find_A1B->second, 0.1);

    }

    TEST(Scenarios_Inflation_ImplicitSymbols, ITE_222) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 2}, {{0, 1}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        const oper_name_t op_A = context.Observables()[0].operator_offset;
        const oper_name_t op_B = context.Observables()[1].operator_offset;
        const oper_name_t op_C = context.Observables()[2].operator_offset;

        auto [id1, momentMatrix1] = ims.create_moment_matrix(1);
        auto [id2, momentMatrix2] = ims.create_moment_matrix(2);
        const auto& implSym = ims.ImplicitSymbolTable();

        auto id_ePtr = symbols.where(OperatorSequence::Identity(context));
        ASSERT_NE(id_ePtr, nullptr);
        auto id_e  = id_ePtr->Id();

        auto id_APtr = symbols.where(OperatorSequence{{op_A}, context});
        ASSERT_NE(id_APtr, nullptr);
        auto id_A  = id_APtr->Id();

        auto id_BPtr = symbols.where(OperatorSequence{{op_B}, context});
        ASSERT_NE(id_BPtr, nullptr);
        auto id_B  = id_BPtr->Id();

        auto id_CPtr = symbols.where(OperatorSequence{{op_C}, context});
        ASSERT_NE(id_CPtr, nullptr);
        auto id_C  = id_CPtr->Id();

        auto id_ABPtr = symbols.where(OperatorSequence{{op_A, op_B}, context});
        ASSERT_NE(id_ABPtr, nullptr);
        auto id_AB  = id_ABPtr->Id();

        auto id_ACPtr = symbols.where(OperatorSequence{{op_A, op_C}, context});
        ASSERT_NE(id_ACPtr, nullptr);
        auto id_AC = id_ACPtr->Id();

        auto id_BCPtr = symbols.where(OperatorSequence{{op_B, op_C}, context});
        ASSERT_NE(id_BCPtr, nullptr);
        auto id_BC = id_BCPtr->Id();

        auto id_ABCPtr = symbols.where(OperatorSequence{{op_A, op_B, op_C}, context});
        ASSERT_NE(id_ABCPtr, nullptr);
        auto id_ABC = id_ABCPtr->Id();

        std::set all_ids{id_e, id_A, id_B, id_C, id_AB, id_AC, id_BC, id_ABC};
        ASSERT_EQ(all_ids.size(), 8);

        std::vector<OVIndex> mmts{{0, 0}, {1, 0}, {2,0}};
        std::vector<double> distribution{0.5, 0, 0, 0, 0, 0, 0, 0.5}; // p(000) = p(111) = 0.5

        auto explicit_form = implSym.implicit_to_explicit(mmts, distribution);
        ASSERT_EQ(explicit_form.size(), 8);

        auto find_e = explicit_form.find(id_e);
        ASSERT_NE(find_e, explicit_form.end());
        auto find_A = explicit_form.find(id_A);
        ASSERT_NE(find_A, explicit_form.end());
        auto find_B = explicit_form.find(id_B);
        ASSERT_NE(find_B, explicit_form.end());
        auto find_C = explicit_form.find(id_C);
        ASSERT_NE(find_C, explicit_form.end());
        auto find_AB = explicit_form.find(id_AB);
        ASSERT_NE(find_AB, explicit_form.end());
        auto find_AC = explicit_form.find(id_AC);
        ASSERT_NE(find_AC, explicit_form.end());
        auto find_BC = explicit_form.find(id_BC);
        ASSERT_NE(find_BC, explicit_form.end());
        auto find_ABC = explicit_form.find(id_ABC);
        ASSERT_NE(find_ABC, explicit_form.end());

        EXPECT_FLOAT_EQ(find_e->second,   1.0);
        EXPECT_FLOAT_EQ(find_A->second,   0.5);
        EXPECT_FLOAT_EQ(find_B->second,   0.5);
        EXPECT_FLOAT_EQ(find_C->second,   0.5);
        EXPECT_FLOAT_EQ(find_AB->second,  0.5);
        EXPECT_FLOAT_EQ(find_AC->second,  0.5);
        EXPECT_FLOAT_EQ(find_BC->second,  0.5);
        EXPECT_FLOAT_EQ(find_ABC->second,  0.5);
    }


}