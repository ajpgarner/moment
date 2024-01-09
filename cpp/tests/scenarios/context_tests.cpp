/**
 * context_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "dictionary/operator_sequence.h"
#include "dictionary/raw_polynomial.h"


namespace Moment::Tests {
    TEST(Scenarios_Context, Construct_Empty) {
        Context context{0};
        ASSERT_EQ(context.size(), 0);
        ASSERT_TRUE(context.empty());

        EXPECT_EQ(context.size(), 0);
    }

    TEST(Scenarios_Context, MultiplyRaw_SingleSingleScalar) {
        Context context{3};
        RawPolynomial rpA;
        rpA.emplace_back(OperatorSequence::Identity(context), {2.0, 0.0});
        EXPECT_EQ(rpA.size(), 1);

        RawPolynomial rpB;
        rpB.emplace_back(OperatorSequence::Identity(context), {3.0, 0.0});
        EXPECT_EQ(rpB.size(), 1);

        const auto result = context.multiply(rpA, rpB);
        ASSERT_EQ(result.size(), 1);
        EXPECT_EQ(result[0].sequence, OperatorSequence::Identity(context));
        EXPECT_EQ(result[0].weight, std::complex(6.0, 0.0));
    }

    TEST(Scenarios_Context, MultiplyRaw_SingleSingle) {
        Context context{3};
        RawPolynomial rpA;
        rpA.emplace_back(OperatorSequence{{0}, context}, {2.0, 0.0});
        EXPECT_EQ(rpA.size(), 1);

        RawPolynomial rpB;
        rpB.emplace_back(OperatorSequence{{1}, context}, {3.0, 0.0});
        EXPECT_EQ(rpB.size(), 1);

        const auto result = context.multiply(rpA, rpB);
        ASSERT_EQ(result.size(), 1);
        EXPECT_EQ(result[0].sequence, (OperatorSequence{{0, 1}, context}));
        EXPECT_EQ(result[0].weight, std::complex(6.0, 0.0));
    }


    TEST(Scenarios_Context, MultiplyRaw_ScalarPoly) {
        Context context{3};
        RawPolynomial rpA;
        rpA.emplace_back(OperatorSequence::Identity(context), {0.0, 1.0});
        ASSERT_EQ(rpA.size(), 1);

        RawPolynomial rpB;
        rpB.emplace_back(OperatorSequence{{0}, context}, {2.0, 0.0});
        rpB.emplace_back(OperatorSequence{{1}, context}, {3.0, 0.0});
        ASSERT_EQ(rpB.size(), 2);

        const auto result = context.multiply(rpA, rpB);
        ASSERT_EQ(result.size(), 2);
        EXPECT_EQ(result[0].sequence, (OperatorSequence{{0}, context}));
        EXPECT_EQ(result[0].weight, std::complex(0.0, 2.0));
        EXPECT_EQ(result[1].sequence, (OperatorSequence{{1}, context}));
        EXPECT_EQ(result[1].weight, std::complex(0.0, 3.0));
    }

    TEST(Scenarios_Context, MultiplyRaw_PolyPoly) {
        Context context{4};
        RawPolynomial rpA;
        rpA.emplace_back(OperatorSequence{{0}, context}, {2.0, 0.0});
        rpA.emplace_back(OperatorSequence{{1}, context}, {3.0, 0.0});
        ASSERT_EQ(rpA.size(), 2);

        RawPolynomial rpB;
        rpB.emplace_back(OperatorSequence{{2}, context}, {5.0, 0.0});
        rpB.emplace_back(OperatorSequence{{3}, context}, {7.0, 0.0});
        ASSERT_EQ(rpB.size(), 2);

        const auto result = context.multiply(rpA, rpB);
        ASSERT_EQ(result.size(), 4);
        EXPECT_EQ(result[0].sequence, (OperatorSequence{{0, 2}, context}));
        EXPECT_EQ(result[0].weight, std::complex(10.0, 0.0));
        EXPECT_EQ(result[1].sequence, (OperatorSequence{{0, 3}, context}));
        EXPECT_EQ(result[1].weight, std::complex(14.0, 0.0));
        EXPECT_EQ(result[2].sequence, (OperatorSequence{{1, 2}, context}));
        EXPECT_EQ(result[2].weight, std::complex(15.0, 0.0));
        EXPECT_EQ(result[3].sequence, (OperatorSequence{{1, 3}, context}));
        EXPECT_EQ(result[3].weight, std::complex(21.0, 0.0));
    }

    TEST(Scenarios_Context, MultiplyRaw_Binomial) {
        Context context{4};
        RawPolynomial rpA;
        rpA.emplace_back(OperatorSequence::Identity(context), {1.0, 0.0});
        rpA.emplace_back(OperatorSequence{{0}, context}, {1.0, 0.0});
        ASSERT_EQ(rpA.size(), 2);

        const auto result = context.multiply(rpA, rpA);
        ASSERT_EQ(result.size(), 3);
        EXPECT_EQ(result[0].sequence, OperatorSequence::Identity(context));
        EXPECT_EQ(result[0].weight, std::complex(1.0, 0.0));
        EXPECT_EQ(result[1].sequence, (OperatorSequence{{0}, context}));
        EXPECT_EQ(result[1].weight, std::complex(2.0, 0.0));
        EXPECT_EQ(result[2].sequence, (OperatorSequence{{0, 0}, context}));
        EXPECT_EQ(result[2].weight, std::complex(1.0, 0.0));
    }
}