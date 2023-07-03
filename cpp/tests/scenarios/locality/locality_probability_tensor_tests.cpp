/**
 * locality_implicit_symbols_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_probability_tensor.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../implicit_symbol_test_helpers.h"

#include <map>
#include <string>
#include <vector>

namespace Moment::Tests {
    namespace {
        void check_ite_mmt(const std::string& desc,
                           const std::map<symbol_name_t, double>& actual,
                           const std::vector<symbol_name_t>& expected_symbols,
                           const std::vector<double>& expected_probabilities) {

            ASSERT_EQ(expected_symbols.size(), expected_probabilities.size()) << desc;

            EXPECT_EQ(actual.size(), expected_symbols.size()) << desc;

            for (size_t index = 0; index < expected_symbols.size(); ++index) {
                auto find_sym = actual.find(expected_symbols[index]);
                ASSERT_NE(find_sym, actual.end()) << desc
                    << ", symbol = " << expected_symbols[index] << ", index = " << index;
                EXPECT_FLOAT_EQ(find_sym->second, expected_probabilities[index])
                    << desc << ", symbol = " << expected_symbols[index] << ", index = " << index;
            }
        }
    }

    using namespace Moment::Locality;

    TEST(Scenarios_Locality_ImplicitSymbols, Empty) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>()};
        auto [eid, emptyMM] = system.create_moment_matrix(1);
        system.RefreshCollinsGisin();

        LocalityProbabilityTensor pt{system.CollinsGisin(), system.localityContext};

        // 0-dimensional tensor.
        EXPECT_THROW([[maybe_unused]] const auto& polys = pt.CGPolynomials(), errors::BadPTError);
    }

//    TEST(Scenarios_Locality_ImplicitSymbols, OnePartyOneMmt) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 1, 3))};
//        const auto& context = system.localityContext;
//
//        const auto& alice = context.Parties[0];
//        ASSERT_EQ(alice.Measurements.size(), 1);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
//        auto where_a0 = momentMatrix.symbols.where(alice_a0);
//        ASSERT_NE(where_a0, nullptr);
//        const auto& alice_a1 = OperatorSequence({alice.measurement_outcome(0,1)}, context);
//        auto where_a1 = momentMatrix.symbols.where(alice_a1);
//        ASSERT_NE(where_a1, nullptr);
//        ASSERT_NE(where_a0, where_a1);
//
//        const auto& implSym = system.ImplicitSymbolTable();
//        EXPECT_EQ(implSym.MaxSequenceLength, 1);
//
//        std::vector<size_t> indices{0};
//        auto pmoSpan = implSym.get(indices);
//        ASSERT_FALSE(pmoSpan.empty());
//        ASSERT_EQ(pmoSpan.size(), 3);
//
//        EXPECT_EQ(pmoSpan[0].symbol_id, where_a0->Id());
//        ASSERT_EQ(pmoSpan[0].expression.size(), 1);
//        EXPECT_EQ(pmoSpan[0].expression[0].id, where_a0->Id());
//        EXPECT_EQ(pmoSpan[0].expression[0].factor, 1.0);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &pmoSpan[0]);
//
//        EXPECT_EQ(pmoSpan[1].symbol_id, where_a1->Id());
//        ASSERT_EQ(pmoSpan[1].expression.size(), 1);
//        EXPECT_EQ(pmoSpan[1].expression[0].id, where_a1->Id());
//        EXPECT_EQ(pmoSpan[1].expression[0].factor, 1.0);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &pmoSpan[1]);
//
//        EXPECT_EQ(pmoSpan[2].symbol_id, -1);
//        ASSERT_EQ(pmoSpan[2].expression.size(), 3);
//        EXPECT_EQ(pmoSpan[2].expression[0].id, 1);
//        EXPECT_EQ(pmoSpan[2].expression[0].factor, 1.0);
//        EXPECT_EQ(pmoSpan[2].expression[1].id, where_a0->Id());
//        EXPECT_EQ(pmoSpan[2].expression[1].factor, -1.0);
//        EXPECT_EQ(pmoSpan[2].expression[2].id, where_a1->Id());
//        EXPECT_EQ(pmoSpan[2].expression[2].factor, -1.0);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,2}}), &pmoSpan[2]);
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, OnePartyTwoMmt) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 2, 2))};
//        const auto& context = system.localityContext;
//        const auto& alice = context.Parties[0];
//        ASSERT_EQ(alice.Measurements.size(), 2);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
//
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
//        auto where_a0 = momentMatrix.symbols.where(alice_a0);
//        ASSERT_NE(where_a0, nullptr);
//        const auto& alice_b0 = OperatorSequence({alice.measurement_outcome(1,0)}, context);
//        auto where_b0 = momentMatrix.symbols.where(alice_b0);
//        ASSERT_NE(where_b0, nullptr);
//        ASSERT_NE(where_a0, where_b0);
//
//        const auto& implSym = system.ImplicitSymbolTable();
//        EXPECT_EQ(implSym.MaxSequenceLength, 1);
//
//        auto spanA = implSym.get({0});
//        test2Mmt(spanA, 1, where_a0->Id(), "a0");
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &spanA[0]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &spanA[1]);
//
//        auto spanB = implSym.get({1});
//        test2Mmt(spanB, 1, where_b0->Id(), "b0");
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,1,0}}), &spanB[0]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,1,1}}), &spanB[1]);
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, TwoPartyOneMmtEach) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
//        const auto& context = system.localityContext;
//
//        ASSERT_EQ(context.Parties.size(), 2);
//        const auto& alice = context.Parties[0];
//        const auto& bob = context.Parties[1];
//        ASSERT_EQ(alice.Measurements.size(), 1);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements.size(), 1);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
//        auto where_a0 = momentMatrix.symbols.where(alice_a0);
//        ASSERT_NE(where_a0, nullptr);
//        const auto& bob_a0 = OperatorSequence({bob.measurement_outcome(0,0)}, context);
//        auto where_b0 = momentMatrix.symbols.where(bob_a0);
//        ASSERT_NE(where_b0, nullptr);
//        ASSERT_NE(where_a0, where_b0);
//        const auto& alice_a0_bob_a0 = OperatorSequence({alice.measurement_outcome(0,0), bob.measurement_outcome(0,0)},
//                                                       context);
//        auto where_alice_bob = momentMatrix.symbols.where(alice_a0_bob_a0);
//        ASSERT_NE(where_alice_bob, nullptr);
//        ASSERT_NE(where_alice_bob, where_a0);
//        ASSERT_NE(where_alice_bob, where_b0);
//
//        const auto& implSym = system.ImplicitSymbolTable();
//        EXPECT_EQ(implSym.MaxSequenceLength, 2);
//
//        // Alice a
//        auto spanA = implSym.get({0});
//        test2Mmt(spanA, 1, where_a0->Id(), "a0");
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &spanA[0]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &spanA[1]);
//
//        // Bob b
//        auto spanB = implSym.get({1});
//        test2Mmt(spanB, 1, where_b0->Id(), "b0");
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{1,0,0}}), &spanB[0]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{1,0,1}}), &spanB[1]);
//
//        // Alice a, Bob b
//        auto spanAB = implSym.get({0, 1});
//        test22JoinMmt(spanAB, 1, where_a0->Id(), where_b0->Id(), where_alice_bob->Id(), "AB");
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}, PMOIndex{1,0,0}}), &spanAB[0]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}, PMOIndex{1,0,1}}), &spanAB[1]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}, PMOIndex{1,0,0}}), &spanAB[2]);
//        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}, PMOIndex{1,0,1}}), &spanAB[3]);
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, CHSH) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
//        const auto& context = system.localityContext;
//
//        ASSERT_EQ(context.Parties.size(), 2);
//        const auto &alice = context.Parties[0];
//        const auto &bob = context.Parties[1];
//        ASSERT_EQ(alice.Measurements.size(), 2);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements.size(), 2);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        auto A0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
//                                                                      context))->Id();
//        auto A1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1, 0)},
//                                                                      context))->Id();
//        auto B0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
//                                                                      context))->Id();
//        auto B1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1, 0)},
//                                                                      context))->Id();
//        auto A0B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
//                                                                         bob.measurement_outcome(0, 0)},
//                                                                        context))->Id();
//        auto A0B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
//                                                                         bob.measurement_outcome(1, 0)},
//                                                                        context))->Id();
//        auto A1B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
//                                                                         bob.measurement_outcome(0, 0)},
//                                                                        context))->Id();
//        auto A1B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
//                                                                         bob.measurement_outcome(1, 0)},
//                                                                        context))->Id();
//        const auto& implSym = system.ImplicitSymbolTable();
//
//
//        auto spanA0 = implSym.get({0});
//        test2Mmt(spanA0, 1, A0, "A0");
//
//        auto spanA1 = implSym.get({1});
//        test2Mmt(spanA1, 1, A1, "A1");
//
//        auto spanB0 = implSym.get({2});
//        test2Mmt(spanB0, 1, B0, "B0");
//
//        auto spanB1 = implSym.get({3});
//        test2Mmt(spanB1, 1, B1, "B1");
//
//        // Alice a, Bob b
//        auto spanA0B0 = implSym.get({0, 2});
//        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");
//
//        auto spanA0B1 = implSym.get({0, 3});
//        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");
//
//        auto spanA1B0 = implSym.get({1, 2});
//        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");
//
//        auto spanA1B1 = implSym.get({1, 3});
//        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, Tripartite322) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
//        const auto& context = system.localityContext;
//
//        ASSERT_EQ(context.Parties.size(), 3);
//        const auto& alice = context.Parties[0];
//        const auto& bob = context.Parties[1];
//        const auto& charlie = context.Parties[2];
//        ASSERT_EQ(alice.Measurements.size(), 2);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements.size(), 2);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(charlie.Measurements.size(), 2);
//        ASSERT_EQ(charlie.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(charlie.Measurements[1].num_outcomes, 2);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(2);
//
//        auto A0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto A1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0)},
//                                                                       context))->Id();
//        auto B0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto B1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0)},
//                                                                       context))->Id();
//        auto C0 = momentMatrix.symbols.where(OperatorSequence({charlie.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto C1 = momentMatrix.symbols.where(OperatorSequence({charlie.measurement_outcome(1,0)},
//                                                                       context))->Id();
//
//        auto A0B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         bob.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A0B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         bob.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A0C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A0C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A1B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         bob.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A1B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         bob.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A1C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A1C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//
//        auto B0C0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto B0C1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto B1C0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto B1C1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//
//        auto A0B0C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                           bob.measurement_outcome(0,0),
//                                                                           charlie.measurement_outcome(0,0)},
//                                                                        context))->Id();
//        auto A0B0C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                           bob.measurement_outcome(0,0),
//                                                                           charlie.measurement_outcome(1,0)},
//                                                                        context))->Id();
//        auto A0B1C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                           bob.measurement_outcome(1,0),
//                                                                           charlie.measurement_outcome(0,0)},
//                                                                        context))->Id();
//        auto A0B1C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                           bob.measurement_outcome(1,0),
//                                                                           charlie.measurement_outcome(1,0)},
//                                                                        context))->Id();
//        auto A1B0C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                           bob.measurement_outcome(0,0),
//                                                                           charlie.measurement_outcome(0,0)},
//                                                                        context))->Id();
//        auto A1B0C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                           bob.measurement_outcome(0,0),
//                                                                           charlie.measurement_outcome(1,0)},
//                                                                        context))->Id();
//        auto A1B1C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                           bob.measurement_outcome(1,0),
//                                                                           charlie.measurement_outcome(0,0)},
//                                                                        context))->Id();
//        auto A1B1C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                           bob.measurement_outcome(1,0),
//                                                                           charlie.measurement_outcome(1,0)},
//                                                                        context))->Id();
//
//        const auto& implSym = system.ImplicitSymbolTable();
//
//        // MONOPARTITE TESTS:
//        auto spanA0 = implSym.get({0});
//        test2Mmt(spanA0, 1, A0, "A0");
//
//        auto spanA1 = implSym.get({1});
//        test2Mmt(spanA1, 1, A1, "A1");
//
//        auto spanB0 = implSym.get({2});
//        test2Mmt(spanB0, 1, B0, "B0");
//
//        auto spanB1 = implSym.get({3});
//        test2Mmt(spanB1, 1, B1, "B1");
//
//        auto spanC0 = implSym.get({4});
//        test2Mmt(spanC0, 1, C0, "C0");
//
//        auto spanC1 = implSym.get({5});
//        test2Mmt(spanC1, 1, C1, "C1");
//
//        // BIPARTITE TESTS:
//        const auto spanA0B0 = implSym.get({0, 2});
//        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");
//
//        const auto spanA0B1 = implSym.get({0, 3});
//        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");
//
//        const auto spanA1B0 = implSym.get({1, 2});
//        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");
//
//        const auto spanA1B1 = implSym.get({1, 3});
//        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");
//
//        const auto spanA0C0 = implSym.get({0, 4});
//        test22JoinMmt(spanA0C0, 1, A0, C0, A0C0, "A0C0");
//
//        const auto spanA0C1 = implSym.get({0, 5});
//        test22JoinMmt(spanA0C1, 1, A0, C1, A0C1, "A0C1");
//
//        const auto spanA1C0 = implSym.get({1, 4});
//        test22JoinMmt(spanA1C0, 1, A1, C0, A1C0, "A1C0");
//
//        const auto spanA1C1 = implSym.get({1, 5});
//        test22JoinMmt(spanA1C1, 1, A1, C1, A1C1, "A1C1");
//
//        const auto spanB0C0 = implSym.get({2, 4});
//        test22JoinMmt(spanB0C0, 1, B0, C0, B0C0, "B0C0");
//
//        const auto spanB0C1 = implSym.get({2, 5});
//        test22JoinMmt(spanB0C1, 1, B0, C1, B0C1, "B0C1");
//
//        const auto spanB1C0 = implSym.get({3, 4});
//        test22JoinMmt(spanB1C0, 1, B1, C0, B1C0, "B0C0");
//
//        const auto spanB1C1 = implSym.get({3, 5});
//        test22JoinMmt(spanB1C1, 1, B1, C1, B1C1, "B1C1");
//
//        // TRIPARTITE TESTS
//        const auto spanA0B0C0 = implSym.get({0, 2, 4});
//        test222JoinMmt(spanA0B0C0, 1, A0, B0, C0, A0B0, A0C0, B0C0, A0B0C0, "A0B0C0");
//
//        const auto spanA0B0C1 = implSym.get({0, 2, 5});
//        test222JoinMmt(spanA0B0C1, 1, A0, B0, C1, A0B0, A0C1, B0C1, A0B0C1, "A0B0C1");
//
//        const auto spanA0B1C0 = implSym.get({0, 3, 4});
//        test222JoinMmt(spanA0B1C0, 1, A0, B1, C0, A0B1, A0C0, B1C0, A0B1C0, "A0B1C0");
//
//        const auto spanA0B1C1 = implSym.get({0, 3, 5});
//        test222JoinMmt(spanA0B1C1, 1, A0, B1, C1, A0B1, A0C1, B1C1, A0B1C1, "A0B1C1");
//
//        const auto spanA1B0C0 = implSym.get({1, 2, 4});
//        test222JoinMmt(spanA1B0C0, 1, A1, B0, C0, A1B0, A1C0, B0C0, A1B0C0, "A1B0C0");
//
//        const auto spanA1B0C1 = implSym.get({1, 2, 5});
//        test222JoinMmt(spanA1B0C1, 1, A1, B0, C1, A1B0, A1C1, B0C1, A1B0C1, "A1B0C1");
//
//        const auto spanA1B1C0 = implSym.get({1, 3, 4});
//        test222JoinMmt(spanA1B1C0, 1, A1, B1, C0, A1B1, A1C0, B1C0, A1B1C0, "A1B1C0");
//
//        const auto spanA1B1C1 = implSym.get({1, 3, 5});
//        test222JoinMmt(spanA1B1C1, 1, A1, B1, C1, A1B1, A1C1, B1C1, A1B1C1, "A1B1C1");
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, Tripartite322_LowerMoment) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
//        const auto& context = system.localityContext;
//
//        ASSERT_EQ(context.Parties.size(), 3);
//        const auto& alice = context.Parties[0];
//        const auto& bob = context.Parties[1];
//        const auto& charlie = context.Parties[2];
//        ASSERT_EQ(alice.Measurements.size(), 2);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements.size(), 2);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(charlie.Measurements.size(), 2);
//        ASSERT_EQ(charlie.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(charlie.Measurements[1].num_outcomes, 2);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        auto A0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto A1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0)},
//                                                                       context))->Id();
//        auto B0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto B1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0)},
//                                                                       context))->Id();
//        auto C0 = momentMatrix.symbols.where(OperatorSequence({charlie.measurement_outcome(0,0)},
//                                                                       context))->Id();
//        auto C1 = momentMatrix.symbols.where(OperatorSequence({charlie.measurement_outcome(1,0)},
//                                                                       context))->Id();
//
//        auto A0B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         bob.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A0B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         bob.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A0C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A0C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A1B0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         bob.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A1B1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         bob.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto A1C0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto A1C1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//
//        auto B0C0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto B0C1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//        auto B1C0 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(0,0)},
//                                                                         context))->Id();
//        auto B1C1 = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
//                                                                         charlie.measurement_outcome(1,0)},
//                                                                         context))->Id();
//
//        const auto& implSym = system.ImplicitSymbolTable();
//
//        // MONOPARTITE TESTS:
//        auto spanA0 = implSym.get({0});
//        test2Mmt(spanA0, 1, A0, "A0");
//
//        auto spanA1 = implSym.get({1});
//        test2Mmt(spanA1, 1, A1, "A1");
//
//        auto spanB0 = implSym.get({2});
//        test2Mmt(spanB0, 1, B0, "B0");
//
//        auto spanB1 = implSym.get({3});
//        test2Mmt(spanB1, 1, B1, "B1");
//
//        auto spanC0 = implSym.get({4});
//        test2Mmt(spanC0, 1, C0, "C0");
//
//        auto spanC1 = implSym.get({5});
//        test2Mmt(spanC1, 1, C1, "C1");
//
//        // BIPARTITE TESTS:
//        const auto spanA0B0 = implSym.get({0, 2});
//        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");
//
//        const auto spanA0B1 = implSym.get({0, 3});
//        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");
//
//        const auto spanA1B0 = implSym.get({1, 2});
//        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");
//
//        const auto spanA1B1 = implSym.get({1, 3});
//        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");
//
//        const auto spanA0C0 = implSym.get({0, 4});
//        test22JoinMmt(spanA0C0, 1, A0, C0, A0C0, "A0C0");
//
//        const auto spanA0C1 = implSym.get({0, 5});
//        test22JoinMmt(spanA0C1, 1, A0, C1, A0C1, "A0C1");
//
//        const auto spanA1C0 = implSym.get({1, 4});
//        test22JoinMmt(spanA1C0, 1, A1, C0, A1C0, "A1C0");
//
//        const auto spanA1C1 = implSym.get({1, 5});
//        test22JoinMmt(spanA1C1, 1, A1, C1, A1C1, "A1C1");
//
//        const auto spanB0C0 = implSym.get({2, 4});
//        test22JoinMmt(spanB0C0, 1, B0, C0, B0C0, "B0C0");
//
//        const auto spanB0C1 = implSym.get({2, 5});
//        test22JoinMmt(spanB0C1, 1, B0, C1, B0C1, "B0C1");
//
//        const auto spanB1C0 = implSym.get({3, 4});
//        test22JoinMmt(spanB1C0, 1, B1, C0, B1C0, "B0C0");
//
//        const auto spanB1C1 = implSym.get({3, 5});
//        test22JoinMmt(spanB1C1, 1, B1, C1, B1C1, "B1C1");
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, A13_B12) {
//
//        std::vector<Party> buildParties;
//        buildParties.emplace_back(0, "A", std::vector{Measurement("a", 3)});
//        buildParties.emplace_back(1, "B", std::vector{Measurement("b", 2)});
//
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(std::move(buildParties))};
//        const auto& context = system.localityContext;
//        ASSERT_EQ(context.Parties.size(), 2);
//        const auto &alice = context.Parties[0];
//        const auto &bob = context.Parties[1];
//        ASSERT_EQ(alice.Measurements.size(), 1);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);
//        ASSERT_EQ(bob.Measurements.size(), 1);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//
//        auto A0 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
//                                                                      context))->Id();
//        auto A1 = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 1)},
//                                                                      context))->Id();
//        auto B = momentMatrix.symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
//                                                                      context))->Id();
//
//        auto A0B = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
//                                                                         bob.measurement_outcome(0, 0)},
//                                                                        context))->Id();
//        auto A1B = momentMatrix.symbols.where(OperatorSequence({alice.measurement_outcome(0, 1),
//                                                                         bob.measurement_outcome(0, 0)},
//                                                                        context))->Id();
//
//        const auto& implSym = system.ImplicitSymbolTable();
//
//        // Alice
//        auto spanA = implSym.get({0});
//        test3Mmt(spanA, 1, A0, A1, "A");
//
//        // Bob
//        auto spanB = implSym.get({1});
//        test2Mmt(spanB, 1, B, "B");
//
//        // Alice a, Bob b
//        auto spanAB = implSym.get({0, 1});
//        test32JoinMmt(spanAB, 1, A0, A1, B, A0B, A1B, "AB");
//    }
//
//    TEST(Scenarios_Locality_ImplicitSymbols, ITE_CHSH) {
//        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
//        const auto& context = system.localityContext;
//        ASSERT_EQ(context.Parties.size(), 2);
//        const auto& alice = context.Parties[0];
//        const auto& bob = context.Parties[1];
//
//        ASSERT_EQ(alice.Measurements.size(), 2);
//        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements.size(), 2);
//        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
//        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
//
//        auto op_A0 = alice.measurement_outcome(0, 0);
//        auto op_A1 = alice.measurement_outcome(1, 0);
//        auto op_B0 = bob.measurement_outcome(0, 0);
//        auto op_B1 = bob.measurement_outcome(1, 0);
//        std::set<oper_name_t> all_ops{ op_A0, op_A1, op_B0, op_B1};
//        ASSERT_EQ(all_ops.size(), 4);
//
//        auto [id, momentMatrix] = system.create_moment_matrix(1);
//        const auto& symbols = system.Symbols();
//
//        auto id_ePtr = symbols.where(OperatorSequence::Identity(context));
//        ASSERT_NE(id_ePtr, nullptr);
//        auto id_e  = id_ePtr->Id();
//
//        auto id_A0Ptr = symbols.where(OperatorSequence{{op_A0}, context});
//        ASSERT_NE(id_A0Ptr, nullptr);
//        auto id_A0  = id_A0Ptr->Id();
//
//        auto id_A1Ptr = symbols.where(OperatorSequence{{op_A1}, context});
//        ASSERT_NE(id_A1Ptr, nullptr);
//        auto id_A1  = id_A1Ptr->Id();
//
//        auto id_B0Ptr = symbols.where(OperatorSequence{{op_B0}, context});
//        ASSERT_NE(id_B0Ptr, nullptr);
//        auto id_B0  = id_B0Ptr->Id();
//
//        auto id_B1Ptr = symbols.where(OperatorSequence{{op_B1}, context});
//        ASSERT_NE(id_B1Ptr, nullptr);
//        auto id_B1  = id_B1Ptr->Id();
//
//        auto id_A0B0Ptr = symbols.where(OperatorSequence{{op_A0, op_B0}, context});
//        ASSERT_NE(id_A0B0Ptr, nullptr);
//        auto id_A0B0  = id_A0B0Ptr->Id();
//
//        auto id_A0B1Ptr = symbols.where(OperatorSequence{{op_A0, op_B1}, context});
//        ASSERT_NE(id_A0B0Ptr, nullptr);
//        auto id_A0B1  = id_A0B1Ptr->Id();
//
//        auto id_A1B0Ptr = symbols.where(OperatorSequence{{op_A1, op_B0}, context});
//        ASSERT_NE(id_A1B0Ptr, nullptr);
//        auto id_A1B0  = id_A1B0Ptr->Id();
//
//        auto id_A1B1Ptr = symbols.where(OperatorSequence{{op_A1, op_B1}, context});
//        ASSERT_NE(id_A1B1Ptr, nullptr);
//        auto id_A1B1  = id_A1B1Ptr->Id();
//
//        std::set all_symbols{id_e, id_A0, id_A1, id_B0, id_B1,
//                             id_A0B0, id_A0B1, id_A1B0, id_A1B1};
//        ASSERT_EQ(all_symbols.size(), 9);
//
//        const auto& implSym = system.ImplicitSymbolTable();
//
//        const std::vector<double> distribution{0.1, 0.2, 0.3, 0.4};
//
//        // A0 B0
//        const std::vector<PMIndex> mmts_A0B0{{0, 0}, {1, 0}};
//        auto ef_A0B0 = implSym.implicit_to_explicit(mmts_A0B0, distribution);
//        check_ite_mmt("A0B0", ef_A0B0, {id_e, id_A0, id_B0, id_A0B0}, {1.0, 0.3, 0.4, 0.1});
//
//        // A0 B1
//        const std::vector<PMIndex> mmts_A0B1{{0, 0}, {1, 1}};
//        auto ef_A0B1 = implSym.implicit_to_explicit(mmts_A0B1, distribution);
//        check_ite_mmt("A0B1", ef_A0B1, {id_e, id_A0, id_B1, id_A0B1}, {1.0, 0.3, 0.4, 0.1});
//
//        // A1 B0
//        const std::vector<PMIndex> mmts_A1B0{{0, 1}, {1, 0}};
//        auto ef_A1B0 = implSym.implicit_to_explicit(mmts_A1B0, distribution);
//        check_ite_mmt("A1B0", ef_A1B0, {id_e, id_A1, id_B0, id_A1B0}, {1.0, 0.3, 0.4, 0.1});
//
//        // A1 B1
//        const std::vector<PMIndex> mmts_A1B1{{0, 1}, {1, 1}};
//        auto ef_A1B1 = implSym.implicit_to_explicit(mmts_A1B1, distribution);
//        check_ite_mmt("A1B1", ef_A1B1, {id_e, id_A1, id_B1, id_A1B1}, {1.0, 0.3, 0.4, 0.1});
//
//    }

}