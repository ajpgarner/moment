/**
 * extended_matrix_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "scenarios/inflation/extended_matrix.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include <sstream>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    namespace {
        symbol_name_t find_or_fail(const SymbolTable& symbols, const OperatorSequence& seq) {
            const UniqueSequence * find_ptr = symbols.where(seq);
            if (find_ptr == nullptr) {
                std::stringstream ss;
                ss << "Could not find sequence \"" << seq << "\".";
                throw std::logic_error{ss.str()};
            }
            return find_ptr->Id();
        }

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

        void compare_symbol_matrices(const SymbolicMatrix::SymbolMatrixView& test,
                                     const std::vector<symbol_name_t>& reference) {
            ASSERT_EQ(test.Dimension()*test.Dimension(), reference.size());
            auto refIter = reference.cbegin();
            for (size_t row_counter = 0; row_counter < test.Dimension(); ++row_counter) {
                for (size_t column_counter = 0; column_counter < test.Dimension(); ++column_counter) {
                    EXPECT_EQ(test[row_counter][column_counter].id, *refIter) << "row = " << row_counter
                                                                              << ", col = " << column_counter;
                    ++refIter;
                }
            }
        }
    }


    TEST(Scenarios_Inflation_ExtendedMatrix, Pair_Linked) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {{0, 1}}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        ims.create_moment_matrix(1);
        const auto& base_MM = ims.MomentMatrix(1);

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

        std::vector<symbol_name_t> extension_list{id_A};

        ExtendedMatrix extended_MM{symbols, factors, base_MM, extension_list};

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

        ims.create_moment_matrix(1);
        const auto& base_MM = ims.MomentMatrix(1);

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

        std::vector<symbol_name_t> extension_list{id_A};

        ExtendedMatrix extended_MM{symbols, factors, base_MM, extension_list};

        // new symbols to expect: <A><A>; meanwhile <A><B>=<AB>, so is not a new symbol
        ASSERT_EQ(symbols.size(), 6);
        auto id_A_A = find_or_fail(factors, {id_A, id_A});
        all_symbs.emplace(id_A_A);
        ASSERT_EQ(all_symbs.size(), 6);

        compare_symbol_matrices(extended_MM.SymbolMatrix, {id_e, id_A,   id_B,   id_A,
                                                           id_A, id_A,   id_AB,  id_A_A,
                                                           id_B, id_AB,  id_B,   id_AB,
                                                           id_A, id_A_A, id_AB,  id_A_A});
    }

    TEST(Scenarios_Inflation_ExtendedMatrix, Pair_DoubleExtension) {
        InflationMatrixSystem ims{std::make_unique<InflationContext>(CausalNetwork{{2, 2}, {}}, 1)};
        auto& symbols = ims.Symbols();
        auto& factors = ims.Factors();

        ims.create_moment_matrix(1);
        const auto& base_MM = ims.MomentMatrix(1);

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

        ExtendedMatrix extended_MM{symbols, factors, base_MM, extension_list};

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
}