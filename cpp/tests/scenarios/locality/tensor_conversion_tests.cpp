/**
 * tensor_conversion_tests.cpp
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/tensor_conversion.h"

#include <stdexcept>

namespace Moment::Tests {
    using namespace Moment::Locality;


    TEST(Scenarios_Locality_TensorConversion, BadContext) {
        LocalityContext context(Party::MakeList(2, 2, 3));
        EXPECT_THROW(TensorConvertor{context}, std::logic_error);
    }

    TEST(Scenarios_Locality_TensorConversion, TrivialTwoParty) {
        LocalityContext context(Party::MakeList(2, 1, 2));
        TensorConvertor convertor{context};
        // Expected matrix layout:
        //  1  b
        //  a ab
        ASSERT_EQ(convertor.tensor_info.ElementCount, 4);

        const std::vector<double> fc = { 0.0, -1.0, -1.0, 1.0};
        const std::vector<double> cg = {3.0, -4.0, -4.0, 4.0};

        auto actual_cg = convertor.full_correlator_to_collins_gisin(fc);
        ASSERT_EQ(actual_cg.size(), 4);
        EXPECT_EQ(actual_cg, cg);

        auto actual_fc = convertor.collins_gisin_to_full_correlator(cg);
        ASSERT_EQ(actual_fc.size(), 4);
        EXPECT_EQ(actual_fc, fc);
    }


    TEST(Scenarios_Locality_TensorConversion, CHSH) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        TensorConvertor convertor{context};
        // Expected matrix layout:
        //  1  b
        //  a ab
        ASSERT_EQ(convertor.tensor_info.ElementCount, 9);

        const std::vector<double> fc = { 0.0, 0.0, 0.0,
                                         0.0, 1.0, 1.0,
                                         0.0, 1.0, -1.0};
        const std::vector<double> cg = { 2.0, -4.0,  0.0,
                                        -4.0,  4.0,  4.0,
                                         0.0,  4.0, -4.0};

        auto actual_cg = convertor.full_correlator_to_collins_gisin(fc);
        ASSERT_EQ(actual_cg.size(), 9);
        EXPECT_EQ(actual_cg, cg);

        auto actual_fc = convertor.collins_gisin_to_full_correlator(cg);
        ASSERT_EQ(actual_fc.size(), 9);
        EXPECT_EQ(actual_fc, fc);
    }

    TEST(Scenarios_Locality_TensorConversion, ThreeByTwo) {
        LocalityContext context(Party::MakeList({3, 2}, {2, 2, 2, 2, 2}));
        TensorConvertor convertor{context};
        // Expected matrix layout:
        ASSERT_EQ(convertor.tensor_info.ElementCount, 12);

        // <A1B1> + <A2B2> - <A3B1>:
        const std::vector<double> fc = { 0.0, 0.0, 0.0, 0.0,
                                         0.0, 1.0, 0.0, -1.0,
                                         0.0, 0.0, 1.0, 0.0};
        // 1 + -2 A.a0 + -2 A.b0 + 2 A.c0 + -2 B.b0 + 4 A.a0 B.a0 + 4 A.b0 B.b0 + -4 A.c0 B.a0
        const std::vector<double> cg = { 1.0, -2.0, -2.0,  2.0,
                                         0.0,  4.0,  0.0, -4.0,
                                         -2.0, 0.0,  4.0,  0.0};

        auto actual_cg = convertor.full_correlator_to_collins_gisin(fc);
        ASSERT_EQ(actual_cg.size(), 12);
        EXPECT_EQ(actual_cg, cg);

        auto actual_fc = convertor.collins_gisin_to_full_correlator(cg);
        ASSERT_EQ(actual_fc.size(), 12);
        EXPECT_EQ(actual_fc, fc);
    }



}