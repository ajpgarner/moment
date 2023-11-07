/**
 * sequence_sign_type_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "sequence_sign_type.h"

namespace Moment::Tests {

    TEST(Operators_SequenceSignType, Equality) {
        EXPECT_EQ(SequenceSignType::Positive, SequenceSignType::Positive);
        EXPECT_EQ(SequenceSignType::Negative, SequenceSignType::Negative);
        EXPECT_EQ(SequenceSignType::Imaginary, SequenceSignType::Imaginary);
        EXPECT_EQ(SequenceSignType::NegativeImaginary, SequenceSignType::NegativeImaginary);
    }

    TEST(Operators_SequenceSignType, Inequality) {
        EXPECT_NE(SequenceSignType::Positive, SequenceSignType::Negative);
        EXPECT_NE(SequenceSignType::Positive, SequenceSignType::Imaginary);
        EXPECT_NE(SequenceSignType::Positive, SequenceSignType::NegativeImaginary);
        EXPECT_NE(SequenceSignType::Negative, SequenceSignType::Positive);
        EXPECT_NE(SequenceSignType::Negative, SequenceSignType::Imaginary);
        EXPECT_NE(SequenceSignType::Negative, SequenceSignType::NegativeImaginary);
        EXPECT_NE(SequenceSignType::Imaginary, SequenceSignType::Positive);
        EXPECT_NE(SequenceSignType::Imaginary, SequenceSignType::Negative);
        EXPECT_NE(SequenceSignType::Imaginary, SequenceSignType::NegativeImaginary);
        EXPECT_NE(SequenceSignType::NegativeImaginary, SequenceSignType::Positive);
        EXPECT_NE(SequenceSignType::NegativeImaginary, SequenceSignType::Negative);
        EXPECT_NE(SequenceSignType::NegativeImaginary, SequenceSignType::Imaginary);
    }

    TEST(Operators_SequenceSignType, Conjugate) {
        EXPECT_EQ(SequenceSignType::Positive, conjugate(SequenceSignType::Positive));
        EXPECT_EQ(SequenceSignType::Negative, conjugate(SequenceSignType::Negative));
        EXPECT_EQ(SequenceSignType::NegativeImaginary, conjugate(SequenceSignType::Imaginary));
        EXPECT_EQ(SequenceSignType::Imaginary, conjugate(SequenceSignType::NegativeImaginary));
    }

    TEST(Operators_SequenceSignType, Negate) {
        EXPECT_EQ(SequenceSignType::Negative, negate(SequenceSignType::Positive));
        EXPECT_EQ(SequenceSignType::Positive, negate(SequenceSignType::Negative));
        EXPECT_EQ(SequenceSignType::NegativeImaginary, negate(SequenceSignType::Imaginary));
        EXPECT_EQ(SequenceSignType::Imaginary, negate(SequenceSignType::NegativeImaginary));
    }

    TEST(Operators_SequenceSignType, AreNegatives) {
        EXPECT_FALSE(are_negatives(SequenceSignType::Positive, SequenceSignType::Positive));
        EXPECT_FALSE(are_negatives(SequenceSignType::Positive, SequenceSignType::Imaginary));
        EXPECT_TRUE(are_negatives(SequenceSignType::Positive, SequenceSignType::Negative));
        EXPECT_FALSE(are_negatives(SequenceSignType::Positive, SequenceSignType::NegativeImaginary));

        EXPECT_FALSE(are_negatives(SequenceSignType::Imaginary, SequenceSignType::Positive));
        EXPECT_FALSE(are_negatives(SequenceSignType::Imaginary, SequenceSignType::Imaginary));
        EXPECT_FALSE(are_negatives(SequenceSignType::Imaginary, SequenceSignType::Negative));
        EXPECT_TRUE(are_negatives(SequenceSignType::Imaginary, SequenceSignType::NegativeImaginary));

        EXPECT_TRUE(are_negatives(SequenceSignType::Negative, SequenceSignType::Positive));
        EXPECT_FALSE(are_negatives(SequenceSignType::Negative, SequenceSignType::Imaginary));
        EXPECT_FALSE(are_negatives(SequenceSignType::Negative, SequenceSignType::Negative));
        EXPECT_FALSE(are_negatives(SequenceSignType::Negative, SequenceSignType::NegativeImaginary));

        EXPECT_FALSE(are_negatives(SequenceSignType::NegativeImaginary, SequenceSignType::Positive));
        EXPECT_TRUE(are_negatives(SequenceSignType::NegativeImaginary, SequenceSignType::Imaginary));
        EXPECT_FALSE(are_negatives(SequenceSignType::NegativeImaginary, SequenceSignType::Negative));
        EXPECT_FALSE(are_negatives(SequenceSignType::NegativeImaginary, SequenceSignType::NegativeImaginary));
    }

    TEST(Operators_SequenceSignType, Multiply) {
        EXPECT_EQ(SequenceSignType::Positive * SequenceSignType::Positive, SequenceSignType::Positive);
        EXPECT_EQ(SequenceSignType::Positive * SequenceSignType::Imaginary, SequenceSignType::Imaginary);
        EXPECT_EQ(SequenceSignType::Positive * SequenceSignType::Negative, SequenceSignType::Negative);
        EXPECT_EQ(SequenceSignType::Positive * SequenceSignType::NegativeImaginary, SequenceSignType::NegativeImaginary);

        EXPECT_EQ(SequenceSignType::Imaginary * SequenceSignType::Positive, SequenceSignType::Imaginary);
        EXPECT_EQ(SequenceSignType::Imaginary * SequenceSignType::Imaginary, SequenceSignType::Negative);
        EXPECT_EQ(SequenceSignType::Imaginary * SequenceSignType::Negative, SequenceSignType::NegativeImaginary);
        EXPECT_EQ(SequenceSignType::Imaginary * SequenceSignType::NegativeImaginary, SequenceSignType::Positive);

        EXPECT_EQ(SequenceSignType::Negative * SequenceSignType::Positive, SequenceSignType::Negative);
        EXPECT_EQ(SequenceSignType::Negative * SequenceSignType::Imaginary, SequenceSignType::NegativeImaginary);
        EXPECT_EQ(SequenceSignType::Negative * SequenceSignType::Negative, SequenceSignType::Positive);
        EXPECT_EQ(SequenceSignType::Negative * SequenceSignType::NegativeImaginary, SequenceSignType::Imaginary);

        EXPECT_EQ(SequenceSignType::NegativeImaginary * SequenceSignType::Positive, SequenceSignType::NegativeImaginary);
        EXPECT_EQ(SequenceSignType::NegativeImaginary * SequenceSignType::Imaginary, SequenceSignType::Positive);
        EXPECT_EQ(SequenceSignType::NegativeImaginary * SequenceSignType::Negative, SequenceSignType::Imaginary);
        EXPECT_EQ(SequenceSignType::NegativeImaginary * SequenceSignType::NegativeImaginary, SequenceSignType::Negative);
    }

    TEST(Operators_SequenceSignType, Difference) {
        EXPECT_EQ(difference(SequenceSignType::Positive, SequenceSignType::Positive),
                  SequenceSignType::Positive);
        EXPECT_EQ(difference(SequenceSignType::Positive, SequenceSignType::Imaginary),
                  SequenceSignType::Imaginary);
        EXPECT_EQ(difference(SequenceSignType::Positive, SequenceSignType::Negative),
                  SequenceSignType::Negative);
        EXPECT_EQ(difference(SequenceSignType::Positive, SequenceSignType::NegativeImaginary),
                  SequenceSignType::NegativeImaginary);

        EXPECT_EQ(difference(SequenceSignType::Imaginary, SequenceSignType::Positive),
                  SequenceSignType::NegativeImaginary);
        EXPECT_EQ(difference(SequenceSignType::Imaginary, SequenceSignType::Imaginary),
                  SequenceSignType::Positive);
        EXPECT_EQ(difference(SequenceSignType::Imaginary, SequenceSignType::Negative),
                  SequenceSignType::Imaginary);
        EXPECT_EQ(difference(SequenceSignType::Imaginary, SequenceSignType::NegativeImaginary),
                  SequenceSignType::Negative);

        EXPECT_EQ(difference(SequenceSignType::Negative, SequenceSignType::Positive),
                  SequenceSignType::Negative);
        EXPECT_EQ(difference(SequenceSignType::Negative, SequenceSignType::Imaginary),
                  SequenceSignType::NegativeImaginary);
        EXPECT_EQ(difference(SequenceSignType::Negative, SequenceSignType::Negative),
                  SequenceSignType::Positive);
        EXPECT_EQ(difference(SequenceSignType::Negative, SequenceSignType::NegativeImaginary),
                  SequenceSignType::Imaginary);

        EXPECT_EQ(difference(SequenceSignType::NegativeImaginary, SequenceSignType::Positive),
                  SequenceSignType::Imaginary);
        EXPECT_EQ(difference(SequenceSignType::NegativeImaginary, SequenceSignType::Imaginary),
                  SequenceSignType::Negative);
        EXPECT_EQ(difference(SequenceSignType::NegativeImaginary, SequenceSignType::Negative),
                  SequenceSignType::NegativeImaginary);
        EXPECT_EQ(difference(SequenceSignType::NegativeImaginary, SequenceSignType::NegativeImaginary),
                  SequenceSignType::Positive);
    }



}