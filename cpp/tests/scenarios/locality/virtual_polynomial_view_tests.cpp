/**
 * virtual_polynomial_view_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_probability_tensor.h"

#include "probability/virtual_polynomial_view.h"

#include "symbolic/polynomial_factory.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../probability_tensor_test_helpers.h"

#include <map>
#include <string>
#include <vector>

namespace Moment::Tests {
    using namespace Moment::Locality;

    TEST(Scenarios_Locality_VirtualPolynomialView, Tripartite322) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
        const auto &context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 3);
        const auto &alice = context.Parties[0];
        const oper_name_t a0 = alice.global_offset();
        const oper_name_t a1 = alice.global_offset() + 1;
        const auto &bob = context.Parties[1];
        const oper_name_t b0 = bob.global_offset();
        const oper_name_t b1 = bob.global_offset() + 1;
        const auto &charlie = context.Parties[2];
        const oper_name_t c0 = charlie.global_offset();
        const oper_name_t c1 = charlie.global_offset() + 1;

        // Make 'empty' PT.
        system.RefreshProbabilityTensor();

        const auto& cg = system.CollinsGisin();
        ASSERT_FALSE(cg.HasAllSymbols());
        ASSERT_EQ(cg.Dimensions, (AutoStorageIndex{3, 3, 3}));

        const auto& pt = system.LocalityProbabilityTensor();
        ASSERT_FALSE(pt.HasAllPolynomials());
        ASSERT_EQ(pt.Dimensions, (AutoStorageIndex{5, 5, 5}));

        // Test element a0.0
        const auto pt_a0 = pt(AutoStorageIndex{1, 0, 0}); // a0
        const Polynomial expected_a0{Monomial{static_cast<symbol_name_t>(1+cg.index_to_offset(AutoStorageIndex{1, 0, 0})), 1.0}};
        EXPECT_EQ(pt_a0->cgPolynomial, expected_a0);
        EXPECT_FALSE(pt_a0->hasSymbolPoly);
        const VirtualPolynomialView a0_view{cg, pt_a0->cgPolynomial};
        ASSERT_EQ(a0_view.size(), 1);
        ASSERT_FALSE(a0_view.empty());
        auto a0_view_iter = a0_view.begin();
        ASSERT_NE(a0_view_iter, a0_view.end());
        auto [a0_seq, a0_weight] = *a0_view_iter;
        EXPECT_EQ(a0_seq, OperatorSequence({a0}, context));
        EXPECT_EQ(a0_weight, 1.0);
        ++a0_view_iter;
        EXPECT_EQ(a0_view_iter, a0_view.end());

        // Test element b0.1 = 1 - b0.0
        const auto pt_b01 = pt(AutoStorageIndex{0, 4, 0}); // 1- b0
        const Polynomial expected_b01{
            Monomial{static_cast<symbol_name_t>(1+cg.index_to_offset(AutoStorageIndex{0, 0, 0})), 1.0},
            Monomial{static_cast<symbol_name_t>(1+cg.index_to_offset(AutoStorageIndex{0, 2, 0})), -1.0}
        };
        EXPECT_EQ(pt_b01->cgPolynomial, expected_b01);
        EXPECT_FALSE(pt_b01->hasSymbolPoly);
        const VirtualPolynomialView b01_view{cg, pt_b01->cgPolynomial};
        ASSERT_EQ(b01_view.size(), 2);
        ASSERT_FALSE(b01_view.empty());
        auto b01_view_iter = b01_view.begin();
        ASSERT_NE(b01_view_iter, b01_view.end());
        auto [b01_seq_0, b01_weight_0] = *b01_view_iter;
        EXPECT_EQ(b01_seq_0, OperatorSequence::Identity(context));
        EXPECT_EQ(b01_weight_0, 1.0);
        ++b01_view_iter;
        ASSERT_NE(b01_view_iter, b01_view.end());
        auto [b01_seq_1, b01_weight_1] = *b01_view_iter;
        EXPECT_EQ(b01_seq_1, OperatorSequence({b1}, context));
        EXPECT_EQ(b01_weight_1, -1.0);
        ++b01_view_iter;
        EXPECT_EQ(b01_view_iter, b01_view.end());

    }


}