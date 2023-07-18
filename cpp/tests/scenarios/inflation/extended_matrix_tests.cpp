/**
 * extended_matrix_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "../../symbolic/symbolic_matrix_helpers.h"

#include <sstream>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    namespace {
        symbol_name_t find_or_fail(const FactorTable& factors, std::initializer_list<symbol_name_t> symbols) {

            auto maybe_entry = factors.find_index_by_factors(symbols);
            if (!maybe_entry.has_value()) {
                std::stringstream ss;
                ss << "Could not find symbol with factors ";
                bool done_one = false;
                for (auto f: symbols) {
                    if (done_one) {
                        ss << ", ";
                    } else {
                        done_one = true;
                    }
                    ss << f;
                }
                ss << ".";
                throw std::logic_error{ss.str()};
            }
            return maybe_entry.value();
        }

    }


    TEST(Scenarios_Inflation_ExtendedMatrix, Pair_Linked) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {{0, 1}}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        const auto& base_MM_raw = ims.MomentMatrix(1);
        const auto& base_MM = dynamic_cast<const MonomialMatrix&>(base_MM_raw);

        ASSERT_EQ(symbols.size(), 5);
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A = find_or_fail(symbols, OperatorSequence{{0}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{1}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{0, 1}, ims.Context()});
        std::set all_symbs{id_0, id_e, id_A, id_B, id_AB};
        ASSERT_EQ(all_symbs.size(), 5);

        compare_symbol_matrices(base_MM, {id_e, id_A, id_B,
                                          id_A, id_A, id_AB,
                                          id_B, id_AB, id_B});

        std::vector<symbol_name_t> extension_list{id_A};

        ExtendedMatrix extended_MM{symbols, factors, ims.polynomial_factory().zero_tolerance, base_MM, extension_list};

        // new symbols to expect: <A><A>, <A><B>.
        ASSERT_EQ(symbols.size(), 7);
        auto id_A_A = find_or_fail(factors, {id_A, id_A});
        auto id_A_B = find_or_fail(factors, {id_A, id_B});
        all_symbs.emplace(id_A_A);
        all_symbs.emplace(id_A_B);
        ASSERT_EQ(all_symbs.size(), 7);

        compare_symbol_matrices(extended_MM.SymbolMatrix, {id_e, id_A,   id_B,   id_A,
                                                           id_A, id_A,   id_AB,  id_A_A,
                                                           id_B, id_AB,  id_B,   id_A_B,
                                                           id_A, id_A_A, id_A_B, id_A_A});
    }

    TEST(Scenarios_Inflation_ExtendedMatrix, Pair_Unlinked) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        const auto& base_MM_raw = ims.MomentMatrix(1);
        const auto& base_MM = dynamic_cast<const MonomialMatrix&>(base_MM_raw);

        ASSERT_EQ(symbols.size(), 5);
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A = find_or_fail(symbols, OperatorSequence{{0}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{1}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{0, 1}, ims.Context()});
        std::set all_symbs{id_0, id_e, id_A, id_B, id_AB};
        ASSERT_EQ(all_symbs.size(), 5);

        compare_symbol_matrices(base_MM, {id_e, id_A, id_B,
                                          id_A, id_A, id_AB,
                                          id_B, id_AB, id_B});

        std::vector<symbol_name_t> extension_list{id_A};

        ExtendedMatrix extended_MM{symbols, factors, ims.polynomial_factory().zero_tolerance, base_MM, extension_list};

        // new symbols to expect: <A><A>; meanwhile <A><B>=<AB>, so is not a new symbol
        ASSERT_EQ(symbols.size(), 6);
        auto id_A_A = find_or_fail(factors, {id_A, id_A});
        all_symbs.emplace(id_A_A);
        ASSERT_EQ(all_symbs.size(), 6);

        compare_symbol_matrices(extended_MM, {id_e, id_A,   id_B,   id_A,
                                              id_A, id_A,   id_AB,  id_A_A,
                                              id_B, id_AB,  id_B,   id_AB,
                                              id_A, id_A_A, id_AB,  id_A_A});
    }

    TEST(Scenarios_Inflation_ExtendedMatrix, Pair_DoubleExtension) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        const auto& base_MM_raw = ims.MomentMatrix(1);
        const auto& base_MM = dynamic_cast<const MonomialMatrix&>(base_MM_raw);

        ASSERT_EQ(symbols.size(), 5);
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A = find_or_fail(symbols, OperatorSequence{{0}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{1}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{0, 1}, ims.Context()});
        std::set all_symbs{id_0, id_e, id_A, id_B, id_AB};
        ASSERT_EQ(all_symbs.size(), 5);

        compare_symbol_matrices(base_MM.SymbolMatrix, {id_e, id_A, id_B,
                                                       id_A, id_A, id_AB,
                                                       id_B, id_AB, id_B});

        std::vector<symbol_name_t> extension_list{id_A, id_B};

        ExtendedMatrix extended_MM{symbols, factors, ims.polynomial_factory().zero_tolerance, base_MM, extension_list};

        // new symbols to expect: <A><A>; <B><B> meanwhile <A><B>=<AB>, so is not a new symbol
        ASSERT_EQ(symbols.size(), 7);
        auto id_A_A = find_or_fail(factors, {id_A, id_A});
        auto id_B_B = find_or_fail(factors, {id_B, id_B});
        all_symbs.emplace(id_A_A);
        all_symbs.emplace(id_B_B);
        ASSERT_EQ(all_symbs.size(), 7);

        compare_symbol_matrices(extended_MM.SymbolMatrix, {id_e, id_A,   id_B,   id_A,   id_B,
                                                           id_A, id_A,   id_AB,  id_A_A, id_AB,
                                                           id_B, id_AB,  id_B,   id_AB,  id_B_B,
                                                           id_A, id_A_A, id_AB,  id_A_A, id_AB,
                                                           id_B, id_AB,  id_B_B, id_AB,  id_B_B});
    }


    TEST(Scenarios_Inflation_ExtendedMatrix, MS_UnlinkedPair) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1)};
        auto [mm_index, mm_ref] = ims.MomentMatrix.create(1);

        const auto& base_MM = dynamic_cast<const MonomialMatrix&>(ims.MomentMatrix(1));

        const auto& symbols = ims.Symbols();
        ASSERT_EQ(symbols.size(), 5);
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A = find_or_fail(symbols, OperatorSequence{{0}, ims.Context()});
        auto id_B = find_or_fail(symbols, OperatorSequence{{1}, ims.Context()});
        auto id_AB = find_or_fail(symbols, OperatorSequence{{0, 1}, ims.Context()});
        std::set all_symbs{id_0, id_e, id_A, id_B, id_AB};
        ASSERT_EQ(all_symbs.size(), 5);

        auto [em_index, em_ref] = ims.ExtendedMatrices.create({1, std::vector<symbol_name_t>{id_A}});

        ASSERT_EQ(symbols.size(), 6);
        auto id_A_A = find_or_fail(ims.Factors(), {id_A, id_A});
        all_symbs.emplace(id_A_A);
        ASSERT_EQ(all_symbs.size(), 6);

        compare_symbol_matrices(em_ref.SymbolMatrix, {id_e, id_A,   id_B,   id_A,
                                                      id_A, id_A,   id_AB,  id_A_A,
                                                      id_B, id_AB,  id_B,   id_AB,
                                                      id_A, id_A_A, id_AB,  id_A_A});

        auto [em_second_access, em_sa_ref] = ims.ExtendedMatrices.create({1, std::vector<symbol_name_t>{id_A}});
        EXPECT_EQ(em_second_access, em_index);
        EXPECT_EQ(&em_sa_ref, &em_ref);
    }

    TEST(Scenarios_Inflation_ExtendedMatrix, MS_PairAndScalar) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2, 0}, {{0, 1}}}, 2)};
        auto [mm_index, mm_ref] = ims.MomentMatrix.create(1);
        const auto& base_MM = dynamic_cast<const MonomialMatrix&>(mm_ref);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        oper_name_t op_a0 = context.Observables()[0].variants[0].operator_offset;
        oper_name_t op_a1 = context.Observables()[0].variants[1].operator_offset;
        oper_name_t op_b0 = context.Observables()[1].variants[0].operator_offset;
        oper_name_t op_b1 = context.Observables()[1].variants[1].operator_offset;
        oper_name_t op_c0 = context.Observables()[2].variants[0].operator_offset;

        ASSERT_EQ(symbols.size(), 12) << symbols;
        auto id_0 = find_or_fail(symbols, OperatorSequence::Zero(ims.Context()));
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(ims.Context()));
        auto id_A0 = find_or_fail(symbols, OperatorSequence{{op_a0}, ims.Context()});
        auto id_B0 = find_or_fail(symbols, OperatorSequence{{op_b0}, ims.Context()});
        auto id_C0 = find_or_fail(symbols, OperatorSequence{{op_c0}, ims.Context()});

        auto id_A0A1 = find_or_fail(symbols, OperatorSequence{{op_a0, op_a1}, ims.Context()});
        auto id_A0B0 = find_or_fail(symbols, OperatorSequence{{op_a0, op_b0}, ims.Context()});
        auto id_A0B1 = find_or_fail(symbols, OperatorSequence{{op_a0, op_b1}, ims.Context()});
        auto id_A0C0 = find_or_fail(symbols, OperatorSequence{{op_a0, op_c0}, ims.Context()});
        auto id_B0B1 = find_or_fail(symbols, OperatorSequence{{op_b0, op_b1}, ims.Context()});
        auto id_B0C0 = find_or_fail(symbols, OperatorSequence{{op_b0, op_c0}, ims.Context()});
        auto id_C0C0 = find_or_fail(symbols, OperatorSequence{{op_c0, op_c0}, ims.Context()});


        std::set all_symbs{id_0, id_e, id_A0, id_B0, id_C0,
                            id_A0A1, id_A0B0, id_A0B1, id_A0C0,
                            id_B0B1, id_B0C0, id_C0C0};
        ASSERT_EQ(all_symbs.size(), 12) << symbols;

        auto [em_index, em_ref] = ims.ExtendedMatrices.create({1, std::vector<symbol_name_t>{id_A0}});

    }
}