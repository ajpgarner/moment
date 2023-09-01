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

#include "symbolic/polynomial_factory.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../probability_tensor_test_helpers.h"

#include <map>
#include <string>
#include <vector>

namespace Moment::Tests {
    using namespace Moment::Locality;

    TEST(Scenarios_Locality_ProbabilityTensor, Empty) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>()};
        auto [eid, emptyMM] = system.MomentMatrix.create(1);
        system.RefreshCollinsGisin();

        LocalityProbabilityTensor pt{system};

    }

    TEST(Scenarios_Locality_ProbabilityTensor, OnePartyOneMmt) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 1, 3))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();
        const auto& factory = system.polynomial_factory();

        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);

        auto [id, momentMatrix] = system.MomentMatrix.create(1);
        system.RefreshCollinsGisin();

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const symbol_name_t s_a0 = where_a0->Id();

        const auto& alice_a1 = OperatorSequence({alice.measurement_outcome(0,1)}, context);
        auto where_a1 = symbols.where(alice_a1);
        ASSERT_NE(where_a1, nullptr);
        const symbol_name_t s_a1 = where_a1->Id();
        ASSERT_NE(s_a0, s_a1);

        LocalityProbabilityTensor pt{system};

        ASSERT_EQ(pt.StorageType, TensorStorageType::Explicit);
        ASSERT_EQ(pt.DimensionCount, 1);
        ASSERT_EQ(pt.Dimensions[0], 4); // 0, a0, a1, (a2)
        ASSERT_EQ(pt.ElementCount, 4);

        const auto& data = pt.Data();
        ASSERT_EQ(data.size(), 4);

        ASSERT_TRUE(data[0].hasSymbolPoly);
        EXPECT_EQ(data[0].symbolPolynomial, Polynomial::Scalar(1.0));

        ASSERT_TRUE(data[1].hasSymbolPoly);
        EXPECT_EQ(data[1].symbolPolynomial, factory({Monomial{s_a0, 1.0}}));

        ASSERT_TRUE(data[2].hasSymbolPoly);
        EXPECT_EQ(data[2].symbolPolynomial, factory({Monomial{s_a1, 1.0}}));

        ASSERT_TRUE(data[3].hasSymbolPoly);
        EXPECT_EQ(data[3].symbolPolynomial, factory({Monomial{1, 1.0}, Monomial{s_a0, -1.0}, Monomial{s_a1, -1.0}}));


        LocalityProbabilityTensor virtual_pt{system, TensorStorageType::Virtual};
        ASSERT_EQ(virtual_pt.StorageType, TensorStorageType::Virtual);
        ASSERT_EQ(virtual_pt.DimensionCount, 1);
        ASSERT_EQ(virtual_pt.Dimensions[0], 4); // 0, a0, a1, (a2)
        ASSERT_EQ(virtual_pt.ElementCount, 4);

        const auto range = pt.Splice(AutoStorageIndex{0}, AutoStorageIndex{4});
        auto iter = range.begin();
        ASSERT_NE(iter, range.end());
        ASSERT_TRUE(iter->hasSymbolPoly);
        EXPECT_EQ(iter->symbolPolynomial, Polynomial::Scalar(1.0));

        ++iter;
        ASSERT_NE(iter, range.end());
        ASSERT_TRUE(iter->hasSymbolPoly);
        EXPECT_EQ(iter->symbolPolynomial, factory({Monomial{s_a0, 1.0}}));

        ++iter;
        ASSERT_NE(iter, range.end());
        ASSERT_TRUE(iter->hasSymbolPoly);
        EXPECT_EQ(iter->symbolPolynomial, factory({Monomial{s_a1, 1.0}}));

        ++iter;
        ASSERT_NE(iter, range.end());
        ASSERT_TRUE(iter->hasSymbolPoly);
        EXPECT_EQ(iter->symbolPolynomial, factory({Monomial{1, 1.0}, Monomial{s_a0, -1.0}, Monomial{s_a1, -1.0}}));

        ++iter;
        EXPECT_EQ(iter, range.end());
    }

    TEST(Scenarios_Locality_ProbabilityTensor, OnePartyTwoMmt) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 2, 2))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);


        auto [id, momentMatrix] = system.MomentMatrix.create(1);

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& alice_b0 = OperatorSequence({alice.measurement_outcome(1,0)}, context);
        auto where_b0 = symbols.where(alice_b0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0.symbol, where_b0.symbol);

        system.RefreshProbabilityTensor();
        const auto& pt = system.LocalityProbabilityTensor();

        auto rangeID = pt.measurement_to_range(std::vector<PMIndex>{});
        testIdMmt(rangeID);

        auto rangeA = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        test2Mmt(rangeA, 1, where_a0->Id(), "A0");

        auto rangeB = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}});
        test2Mmt(rangeB, 1, where_b0->Id(), "B0");

    }


    TEST(Scenarios_Locality_ProbabilityTensor, TwoPartyOneMmtEach) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 1);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);


        auto [id, momentMatrix] = system.MomentMatrix.create(1);

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& bob_a0 = OperatorSequence({bob.measurement_outcome(0,0)}, context);
        auto where_b0 = symbols.where(bob_a0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0.symbol, where_b0.symbol);
        const auto& alice_a0_bob_a0 = OperatorSequence({alice.measurement_outcome(0,0), bob.measurement_outcome(0,0)},
                                                       context);
        auto where_alice_bob = symbols.where(alice_a0_bob_a0);
        ASSERT_NE(where_alice_bob, nullptr);
        ASSERT_NE(where_alice_bob.symbol, where_a0.symbol);
        ASSERT_NE(where_alice_bob.symbol, where_b0.symbol);

        system.RefreshProbabilityTensor();
        const auto& pt = system.LocalityProbabilityTensor();

        auto rangeID = pt.measurement_to_range(std::vector<PMIndex>{});
        testIdMmt(rangeID);

        auto rangeA = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        test2Mmt(rangeA, 1, where_a0->Id(), "A0");

        auto rangeB = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}});
        test2Mmt(rangeB, 1, where_b0->Id(), "B0");

        auto rangeAB = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 0}});
        test22JoinMmt(rangeAB, 1, where_a0->Id(), where_b0->Id(), where_alice_bob->Id(), "AB");
    }


    TEST(Scenarios_Locality_ProbabilityTensor, CHSH) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();

        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.MomentMatrix.create(1);

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto A1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0)},
                                                 context))->Id();
        auto B0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto B1 = symbols.where(OperatorSequence({bob.measurement_outcome(1, 0)},
                                                 context))->Id();
        auto A0B0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    bob.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A0B1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    bob.measurement_outcome(1, 0)},
                                                   context))->Id();
        auto A1B0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                    bob.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A1B1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                    bob.measurement_outcome(1, 0)},
                                                   context))->Id();
        system.RefreshProbabilityTensor();
        const auto& pt = system.LocalityProbabilityTensor();

        // Normalization
        auto rangeID = pt.measurement_to_range(std::vector<PMIndex>{});
        testIdMmt(rangeID);

        // Mono-partite
        auto rangeA0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        test2Mmt(rangeA0, 1, A0, "A0");

        auto rangeA1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}});
        test2Mmt(rangeA1, 1, A1, "A1");

        auto rangeB0 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}});
        test2Mmt(rangeB0, 1, B0, "B0");

        auto rangeB1 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 1}});
        test2Mmt(rangeB1, 1, B1, "B1");

        // Bi-partite
        auto rangeA0B0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 0}});
        test22JoinMmt(rangeA0B0, 1, A0, B0, A0B0, "A0B0");

        auto rangeA0B1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 1}});
        test22JoinMmt(rangeA0B1, 1, A0, B1, A0B1, "A0B1");

        auto rangeA1B0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 1, 0}});
        test22JoinMmt(rangeA1B0, 1, A1, B0, A1B0, "A1B0");

        auto rangeA1B1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 1, 1}});
        test22JoinMmt(rangeA1B1, 1, A1, B1, A1B1, "A1B1");
    }


    TEST(Scenarios_Locality_ProbabilityTensor, Tripartite322) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
        const auto &context = system.localityContext;
        const auto& symbols = system.Symbols();

        ASSERT_EQ(context.Parties.size(), 3);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        const auto &charlie = context.Parties[2];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements.size(), 2);
        ASSERT_EQ(charlie.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.MomentMatrix.create(2);

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                              context))->Id();
        auto A1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0)},
                                                              context))->Id();
        auto B0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                              context))->Id();
        auto B1 = symbols.where(OperatorSequence({bob.measurement_outcome(1, 0)},
                                                              context))->Id();
        auto C0 = symbols.where(OperatorSequence({charlie.measurement_outcome(0, 0)},
                                                              context))->Id();
        auto C1 = symbols.where(OperatorSequence({charlie.measurement_outcome(1, 0)},
                                                              context))->Id();

        auto A0B0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                 bob.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto A0B1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                 bob.measurement_outcome(1, 0)},
                                                                context))->Id();
        auto A0C0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                 charlie.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto A0C1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                 charlie.measurement_outcome(1, 0)},
                                                                context))->Id();
        auto A1B0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                 bob.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto A1B1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                 bob.measurement_outcome(1, 0)},
                                                                context))->Id();
        auto A1C0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                 charlie.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto A1C1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                 charlie.measurement_outcome(1, 0)},
                                                                context))->Id();

        auto B0C0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0),
                                                                 charlie.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto B0C1 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0),
                                                                 charlie.measurement_outcome(1, 0)},
                                                                context))->Id();
        auto B1C0 = symbols.where(OperatorSequence({bob.measurement_outcome(1, 0),
                                                                 charlie.measurement_outcome(0, 0)},
                                                                context))->Id();
        auto B1C1 = symbols.where(OperatorSequence({bob.measurement_outcome(1, 0),
                                                                 charlie.measurement_outcome(1, 0)},
                                                                context))->Id();

        auto A0B0C0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                   bob.measurement_outcome(0, 0),
                                                                   charlie.measurement_outcome(0, 0)},
                                                                  context))->Id();
        auto A0B0C1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                   bob.measurement_outcome(0, 0),
                                                                   charlie.measurement_outcome(1, 0)},
                                                                  context))->Id();
        auto A0B1C0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                   bob.measurement_outcome(1, 0),
                                                                   charlie.measurement_outcome(0, 0)},
                                                                  context))->Id();
        auto A0B1C1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                   bob.measurement_outcome(1, 0),
                                                                   charlie.measurement_outcome(1, 0)},
                                                                  context))->Id();
        auto A1B0C0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                   bob.measurement_outcome(0, 0),
                                                                   charlie.measurement_outcome(0, 0)},
                                                                  context))->Id();
        auto A1B0C1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                   bob.measurement_outcome(0, 0),
                                                                   charlie.measurement_outcome(1, 0)},
                                                                  context))->Id();
        auto A1B1C0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                   bob.measurement_outcome(1, 0),
                                                                   charlie.measurement_outcome(0, 0)},
                                                                  context))->Id();
        auto A1B1C1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                   bob.measurement_outcome(1, 0),
                                                                   charlie.measurement_outcome(1, 0)},
                                                                  context))->Id();

        system.RefreshProbabilityTensor();
        const auto &pt = system.LocalityProbabilityTensor();

        // Normalization
        auto rangeID = pt.measurement_to_range(std::vector<PMIndex>{});
        testIdMmt(rangeID);

        // Monopartite
        auto rangeA0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        test2Mmt(rangeA0, 1, A0, "A0");

        auto rangeA1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}});
        test2Mmt(rangeA1, 1, A1, "A1");

        auto rangeB0 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}});
        test2Mmt(rangeB0, 1, B0, "B0");

        auto rangeB1 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 1}});
        test2Mmt(rangeB1, 1, B1, "B1");

        auto rangeC0 = pt.measurement_to_range(std::vector{PMIndex{context, 2, 0}});
        test2Mmt(rangeC0, 1, C0, "C0");

        auto rangeC1 = pt.measurement_to_range(std::vector{PMIndex{context, 2, 1}});
        test2Mmt(rangeC1, 1, C1, "C1");

        // Bipartite

        auto rangeA0B0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 0}});
        test22JoinMmt(rangeA0B0, 1, A0, B0, A0B0, "A0B0");

        auto rangeA0B1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 1}});
        test22JoinMmt(rangeA0B1, 1, A0, B1, A0B1, "A0B1");

        auto rangeA0C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 2, 0}});
        test22JoinMmt(rangeA0C0, 1, A0, C0, A0C0, "A0C0");

        auto rangeA0C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 2, 1}});
        test22JoinMmt(rangeA0C1, 1, A0, C1, A0C1, "A0C1");

        auto rangeA1B0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 1, 0}});
        test22JoinMmt(rangeA1B0, 1, A1, B0, A1B0, "A1B0");

        auto rangeA1B1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 1, 1}});
        test22JoinMmt(rangeA1B1, 1, A1, B1, A1B1, "A1B1");

        auto rangeA1C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 2, 0}});
        test22JoinMmt(rangeA1C0, 1, A1, C0, A1C0, "A1C0");

        auto rangeA1C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1}, PMIndex{context, 2, 1}});
        test22JoinMmt(rangeA1C1, 1, A1, C1, A1C1, "A1C1");

        auto rangeB0C0 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}, PMIndex{context, 2, 0}});
        test22JoinMmt(rangeB0C0, 1, B0, C0, B0C0, "B0C0");

        auto rangeB0C1 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}, PMIndex{context, 2, 1}});
        test22JoinMmt(rangeB0C1, 1, B0, C1, B0C1, "B0C1");
        
        auto rangeB1C0 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 1}, PMIndex{context, 2, 0}});
        test22JoinMmt(rangeB1C0, 1, B1, C0, B1C0, "B1C0");

        auto rangeB1C1 = pt.measurement_to_range(std::vector{PMIndex{context, 1, 1}, PMIndex{context, 2, 1}});
        test22JoinMmt(rangeB1C1, 1, B1, C1, B1C1, "B1C1");
        
        // TRIPARTITE TESTS
        const auto rangeA0B0C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0},
                                                                     PMIndex{context, 1, 0},
                                                                     PMIndex{context, 2, 0}});
        test222JoinMmt(rangeA0B0C0, 1, A0, B0, C0, A0B0, A0C0, B0C0, A0B0C0, "A0B0C0");

        const auto rangeA0B0C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0},
                                                                     PMIndex{context, 1, 0},
                                                                     PMIndex{context, 2, 1}});
        test222JoinMmt(rangeA0B0C1, 1, A0, B0, C1, A0B0, A0C1, B0C1, A0B0C1, "A0B0C1");

        const auto rangeA0B1C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0},
                                                                     PMIndex{context, 1, 1},
                                                                     PMIndex{context, 2, 0}});
        test222JoinMmt(rangeA0B1C0, 1, A0, B1, C0, A0B1, A0C0, B1C0, A0B1C0, "A0B1C0");

        const auto rangeA0B1C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0},
                                                                     PMIndex{context, 1, 1},
                                                                     PMIndex{context, 2, 1}});
        test222JoinMmt(rangeA0B1C1, 1, A0, B1, C1, A0B1, A0C1, B1C1, A0B1C1, "A0B1C1");

        const auto rangeA1B0C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1},
                                                                     PMIndex{context, 1, 0},
                                                                     PMIndex{context, 2, 0}});
        test222JoinMmt(rangeA1B0C0, 1, A1, B0, C0, A1B0, A1C0, B0C0, A1B0C0, "A1B0C0");

        const auto rangeA1B0C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1},
                                                                     PMIndex{context, 1, 0},
                                                                     PMIndex{context, 2, 1}});
        test222JoinMmt(rangeA1B0C1, 1, A1, B0, C1, A1B0, A1C1, B0C1, A1B0C1, "A1B0C1");

        const auto rangeA1B1C0 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1},
                                                                     PMIndex{context, 1, 1},
                                                                     PMIndex{context, 2, 0}});
        test222JoinMmt(rangeA1B1C0, 1, A1, B1, C0, A1B1, A1C0, B1C0, A1B1C0, "A1B1C0");

        const auto rangeA1B1C1 = pt.measurement_to_range(std::vector{PMIndex{context, 0, 1},
                                                                     PMIndex{context, 1, 1},
                                                                     PMIndex{context, 2, 1}});
        test222JoinMmt(rangeA1B1C1, 1, A1, B1, C1, A1B1, A1C1, B1C1, A1B1C1, "A1B1C1");
    }

    TEST(Scenarios_Locality_ProbabilityTensor, A13_B12) {

        std::vector<Party> buildParties;
        buildParties.emplace_back(0, "A", std::vector{Measurement("a", 3)});
        buildParties.emplace_back(1, "B", std::vector{Measurement("b", 2)});

        LocalityMatrixSystem system{std::make_unique<LocalityContext>(std::move(buildParties))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();

        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);
        ASSERT_EQ(bob.Measurements.size(), 1);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);

        auto [id, momentMatrix] = system.MomentMatrix.create(1);

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                                      context))->Id();
        auto A1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 1)},
                                                                      context))->Id();
        auto B = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                                      context))->Id();

        auto A0B = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();
        auto A1B = symbols.where(OperatorSequence({alice.measurement_outcome(0, 1),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();

        system.RefreshProbabilityTensor();
        const auto& pt = system.LocalityProbabilityTensor();

        // Alice
        auto rangeA = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        test3Mmt(rangeA, 1, A0, A1, "A");

        // Bob
        auto rangeB = pt.measurement_to_range(std::vector{PMIndex{context, 1, 0}});
        test2Mmt(rangeB, 1, B, "B");

        // Alice a, Bob b
        auto rangeAB = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 0}});
        test32JoinMmt(rangeAB, 1, A0, A1, B, A0B, A1B, "AB");
    }

    TEST(Scenarios_Locality_ProbabilityTensor, GetOneElem) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        const auto& symbols = system.Symbols();

        system.generate_dictionary(2);
        system.RefreshProbabilityTensor();

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                              context))->Id();
        auto A0B0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                 bob.measurement_outcome(0, 0)},
                                                                context))->Id();
        
        const auto& pt = system.LocalityProbabilityTensor();
        EXPECT_TRUE(pt.HasAllPolynomials());

        const auto view_Aa0 = pt.outcome_to_element(std::vector{PMOIndex{context, 0, 0, 0}}); // a0,0
        ASSERT_TRUE(view_Aa0->hasSymbolPoly);
        EXPECT_EQ(view_Aa0->symbolPolynomial, Polynomial({Monomial{A0, 1.0}}));

        const auto view_Aa0Ba1 = pt.outcome_to_element(std::vector{PMOIndex{context, 0, 0, 0},
                                                                   PMOIndex{context, 1, 0, 1}});
        ASSERT_TRUE(view_Aa0Ba1->hasSymbolPoly);
        EXPECT_EQ(view_Aa0Ba1->symbolPolynomial, Polynomial({Monomial{A0, 1.0}, Monomial{A0B0, -1.0}}));

    }



    TEST(Scenarios_Locality_ProbabilityTensor, CHSH_ExplicitValueRules) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();
        const auto& factory = system.polynomial_factory();

        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.MomentMatrix.create(1);

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto A1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0)},
                                                 context))->Id();
        auto B0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto B1 = symbols.where(OperatorSequence({bob.measurement_outcome(1, 0)},
                                                 context))->Id();
        auto A0B0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    bob.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A0B1 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    bob.measurement_outcome(1, 0)},
                                                   context))->Id();
        auto A1B0 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                    bob.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A1B1 = symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                    bob.measurement_outcome(1, 0)},
                                                   context))->Id();
        system.RefreshProbabilityTensor();
        const auto &pt = system.LocalityProbabilityTensor();

        // A0 measurement
        auto a0_range = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}});
        auto a0_rule_poly = pt.explicit_value_rules(a0_range, std::vector<double>{0.25, 0.75});
        ASSERT_EQ(a0_rule_poly.size(), 2);
        EXPECT_EQ(a0_rule_poly[0], factory({Monomial{1, -0.25}, Monomial{A0, 1.0}}));
        EXPECT_EQ(a0_rule_poly[1], factory({Monomial{1, 0.25}, Monomial{A0, -1.0}}));

        // A0B0 measurement
        auto a0b0_range = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}, PMIndex{context, 1, 0}});
        auto a0b0_rule_poly = pt.explicit_value_rules(a0b0_range, std::vector<double>{0.1, 0.2, 0.3, 0.4});
        ASSERT_EQ(a0b0_rule_poly.size(), 4);
        EXPECT_EQ(a0b0_rule_poly[0], factory({Monomial{1, -0.1}, Monomial{A0B0, 1.0}}));
        EXPECT_EQ(a0b0_rule_poly[1], factory({Monomial{1, -0.2}, Monomial{B0, 1.0}, Monomial{A0B0, -1.0}}));
        EXPECT_EQ(a0b0_rule_poly[2], factory({Monomial{1, -0.3}, Monomial{A0, 1.0}, Monomial{A0B0, -1.0}}));
        EXPECT_EQ(a0b0_rule_poly[3], factory({Monomial{1, 0.6}, Monomial{A0, -1.0},
                                              Monomial{B0, -1.0}, Monomial{A0B0, 1.0}}));

        // P(A0|B1)
        auto a0_given_b10_range = pt.measurement_to_range(std::vector{PMIndex{context, 0, 0}},
                                                         std::vector{PMOIndex{context, 1, 1, 0}});
        auto b10_elem = pt.outcome_to_element(std::vector{PMOIndex{context, 1, 1, 0}});
        auto a0_given_b10_rule_poly = pt.explicit_value_rules(a0_given_b10_range, b10_elem, std::vector<double>{0.1, 0.9});
        ASSERT_EQ(a0_given_b10_rule_poly.size(), 2);
        EXPECT_EQ(a0_given_b10_rule_poly[0], factory({Monomial{B1, -0.1}, Monomial{A0B1, 1.0}}));
        EXPECT_EQ(a0_given_b10_rule_poly[1], factory({Monomial{B1, 0.1}, Monomial{A0B1, -1.0}}));
    }
}