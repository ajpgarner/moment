/**
 * inflation_implicit_symbols_tests.cpp.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/inflation_probability_tensor.h"

#include "matrix/operator_matrix/moment_matrix.h"

#include "../probability_tensor_test_helpers.h"

#include <set>

namespace Moment::Tests {
    using namespace Moment::Inflation;

    TEST(Scenarios_Inflation_ProbabilityTensor, Empty) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{}, {}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        //auto [id, momentMatrix] = ims.MomentMatrix.create(1);

        ims.RefreshProbabilityTensor();
        const auto& implSym = ims.InflationProbabilityTensor();
    }

    TEST(Scenarios_Inflation_ProbabilityTensor, Singleton) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 1);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.MomentMatrix.create(1); // should be [[1 A]; [A A]]
        ims.RefreshProbabilityTensor();
        const auto& pt = ims.InflationProbabilityTensor();

        EXPECT_TRUE(pt.HasAllPolynomials());
        EXPECT_EQ(pt.StorageType, TensorStorageType::Explicit);

        const auto range_id = pt.measurement_to_range(std::vector<OVIndex>{});
        testIdMmt(range_id);

        const auto range_A = pt.measurement_to_range(std::vector{OVIndex{0,0}});
        test2Mmt(range_A, 1, 2);

        const auto elem_A = pt.outcome_to_element(std::vector{OVOIndex{0,0,0}});
        EXPECT_TRUE(elem_A->hasSymbolPoly);
        EXPECT_EQ(elem_A->symbolPolynomial, Polynomial({Monomial{2, 1.0}}));

    }

    TEST(Scenarios_Inflation_ProbabilityTensor, Singleton_Cloned) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.MomentMatrix.create(1);
        ims.RefreshProbabilityTensor();
        const auto& pt = ims.InflationProbabilityTensor();

        const auto& getOne = pt.measurement_to_range(std::vector<OVIndex>{});
        testIdMmt(getOne);

        const auto& getA = pt.measurement_to_range(std::vector<OVIndex>{{0, 0}});
        test2Mmt(getA, 1, 2, "A0");

        const auto& getAprime = pt.measurement_to_range(std::vector<OVIndex>{{0, 1}});
        test2Mmt(getAprime, 1, 2, "A1");

        const auto& getAAprime = pt.measurement_to_range(std::vector<OVIndex>{{0, 0}, {0, 1}});
        test22JoinMmt(getAAprime, 1, 2, 2, 3, "A0A1");
    }


    TEST(Scenarios_Inflation_ProbabilityTensor, CV_cloned) {
        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{0}, {{0}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.MomentMatrix.create(1);
        ims.RefreshProbabilityTensor();
        const auto& pt = ims.InflationProbabilityTensor();

        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();
        ASSERT_EQ(context.size(), 2); // A[0] and A[1].
        auto id_a0 = symbols.where(OperatorSequence({0}, context))->Id(); // A [= A', canonically]
        auto id_a0a1 = symbols.where(OperatorSequence({0, 1}, context))->Id(); // A, A'
        ASSERT_NE(id_a0, id_a0a1);

        const auto& getOne = pt.measurement_to_range(std::vector<OVIndex>{});
        testIdMmt(getOne);

        const auto& getA = pt.measurement_to_range(std::vector<OVIndex>{{0, 0}});
        testSingleCV(getA, id_a0, "A0");

        const auto& getAprime = pt.measurement_to_range(std::vector<OVIndex>{{0, 1}});
        testSingleCV(getAprime, id_a0, "A1");

        const auto& getAAprime = pt.measurement_to_range(std::vector<OVIndex>{{0, 0}, {0, 1}});
        testSingleCV(getAAprime, id_a0a1, "A0A1");
    }
}