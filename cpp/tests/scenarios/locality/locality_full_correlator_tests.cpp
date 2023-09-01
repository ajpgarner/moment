/**
 * locality_full_correlator_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_full_correlator.h"

#include "symbolic/polynomial_factory.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../probability_tensor_test_helpers.h"

#include <map>
#include <string>
#include <vector>

namespace Moment::Tests {
    using namespace Moment::Locality;

    TEST(Scenarios_Locality_FullCorrelator, Empty) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>()};
        system.RefreshCollinsGisin();

        LocalityFullCorrelator fc{system};

    }

    TEST(Scenarios_Locality_FullCorrelator, WrongSizes) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 3))};
        EXPECT_FALSE(system.CanHaveFullCorrelator());
    }

    TEST(Scenarios_Locality_FullCorrelator, CHSH) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        ASSERT_TRUE(system.CanHaveFullCorrelator());
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

        auto [mm_id, momentMatrix] = system.MomentMatrix.create(1);

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

        system.RefreshFullCorrelator();
        const auto& factory = system.polynomial_factory();
        const auto& fc = system.LocalityFullCorrelator();
        ASSERT_TRUE(fc.HasAllPolynomials());

        auto fc_ID = fc.mmt_to_element(std::vector<PMIndex>{});
        EXPECT_EQ(fc_ID->symbolPolynomial, Polynomial::Scalar(1.0));

        auto fc_A0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}});
        EXPECT_EQ(fc_A0->symbolPolynomial, factory({Monomial{A0, 2.0}, Monomial{1, -1.0}}));

        auto fc_A1 = fc.mmt_to_element(std::vector{PMIndex{0, 1}});
        EXPECT_EQ(fc_A1->symbolPolynomial, factory({Monomial{A1, 2.0}, Monomial{1, -1.0}}));

        auto fc_B0 = fc.mmt_to_element(std::vector{PMIndex{1, 0}});
        EXPECT_EQ(fc_B0->symbolPolynomial, factory({Monomial{B0, 2.0}, Monomial{1, -1.0}}));

        auto fc_B1 = fc.mmt_to_element(std::vector{PMIndex{1, 1}});
        EXPECT_EQ(fc_B1->symbolPolynomial, factory({Monomial{B1, 2.0}, Monomial{1, -1.0}}));

        auto fc_A0B0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}, PMIndex{1, 0}});
        EXPECT_EQ(fc_A0B0->symbolPolynomial,
                  factory({Monomial{A0B0, 4.0}, Monomial{A0, -2.0}, Monomial{B0, -2.0}, Monomial{1, 1.0}}));

        auto fc_A1B0 = fc.mmt_to_element(std::vector{PMIndex{0, 1}, PMIndex{1, 0}});
        EXPECT_EQ(fc_A1B0->symbolPolynomial,
                  factory({Monomial{A1B0, 4.0}, Monomial{A1, -2.0}, Monomial{B0, -2.0}, Monomial{1, 1.0}}));

        auto fc_A0B1 = fc.mmt_to_element(std::vector{PMIndex{0, 0}, PMIndex{1, 1}});
        EXPECT_EQ(fc_A0B1->symbolPolynomial,
                  factory({Monomial{A0B1, 4.0}, Monomial{A0, -2.0}, Monomial{B1, -2.0}, Monomial{1, 1.0}}));

        auto fc_A1B1 = fc.mmt_to_element(std::vector{PMIndex{0, 1}, PMIndex{1, 1}});
        EXPECT_EQ(fc_A1B1->symbolPolynomial,
                  factory({Monomial{A1B1, 4.0}, Monomial{A1, -2.0}, Monomial{B1, -2.0}, Monomial{1, 1.0}}));

    }

    TEST(Scenarios_Locality_FullCorrelator, Tripartite) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 1, 2))};
        ASSERT_TRUE(system.CanHaveFullCorrelator());
        const auto& context = system.localityContext;
        const auto& symbols = system.Symbols();

        ASSERT_EQ(context.Parties.size(), 3);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        const auto &charlie = context.Parties[2];

        auto [mm_id, momentMatrix] = system.MomentMatrix.create(2);

        auto A0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto B0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto C0 = symbols.where(OperatorSequence({charlie.measurement_outcome(0, 0)},
                                                 context))->Id();
        auto A0B0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    bob.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A0C0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                    charlie.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto B0C0 = symbols.where(OperatorSequence({bob.measurement_outcome(0, 0),
                                                    charlie.measurement_outcome(0, 0)},
                                                   context))->Id();
        auto A0B0C0 = symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                      bob.measurement_outcome(0, 0),
                                                      charlie.measurement_outcome(0, 0)},
                                                     context))->Id();

        system.RefreshFullCorrelator();
        const auto& factory = system.polynomial_factory();
        const auto& fc = system.LocalityFullCorrelator();
        ASSERT_TRUE(fc.HasAllPolynomials());


        auto fc_ID = fc.mmt_to_element(std::vector<PMIndex>{});
        EXPECT_EQ(fc_ID->symbolPolynomial, Polynomial::Scalar(1.0));

        auto fc_A0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}});
        EXPECT_EQ(fc_A0->symbolPolynomial, factory({Monomial{A0, 2.0}, Monomial{1, -1.0}}));

        auto fc_B0 = fc.mmt_to_element(std::vector{PMIndex{1, 0}});
        EXPECT_EQ(fc_B0->symbolPolynomial, factory({Monomial{B0, 2.0}, Monomial{1, -1.0}}));

        auto fc_C0 = fc.mmt_to_element(std::vector{PMIndex{2, 0}});
        EXPECT_EQ(fc_C0->symbolPolynomial, factory({Monomial{C0, 2.0}, Monomial{1, -1.0}}));

        auto fc_A0B0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}, PMIndex{1, 0}});
        EXPECT_EQ(fc_A0B0->symbolPolynomial,
                  factory({Monomial{A0B0, 4.0}, Monomial{A0, -2.0}, Monomial{B0, -2.0}, Monomial{1, 1.0}}));

        auto fc_A0C0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}, PMIndex{2, 0}});
        EXPECT_EQ(fc_A0C0->symbolPolynomial,
                  factory({Monomial{A0C0, 4.0}, Monomial{A0, -2.0}, Monomial{C0, -2.0}, Monomial{1, 1.0}}));

        auto fc_B0C0 = fc.mmt_to_element(std::vector{PMIndex{1, 0}, PMIndex{2, 0}});
        EXPECT_EQ(fc_B0C0->symbolPolynomial,
                  factory({Monomial{B0C0, 4.0}, Monomial{B0, -2.0}, Monomial{C0, -2.0}, Monomial{1, 1.0}}));

        auto fc_A0B0C0 = fc.mmt_to_element(std::vector{PMIndex{0, 0}, PMIndex{1, 0}, PMIndex{2, 0}});
        EXPECT_EQ(fc_A0B0C0->symbolPolynomial,
                  factory({Monomial{A0B0C0, 8.0}, Monomial{A0B0, -4.0}, Monomial{A0C0, -4.0}, Monomial{B0C0, -4.0},
                           Monomial{A0, +2.0}, Monomial{B0, +2.0}, Monomial{C0, +2.0}, Monomial{1, -1.0}}));

    }

}