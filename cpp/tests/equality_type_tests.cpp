/**
 * equality_type_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "symbolic/equality_type.h"

namespace NPATK::Tests {

    TEST(EqualityType, DistinctValues) {
        EXPECT_NE(EqualityType::none, EqualityType::equal);
        EXPECT_NE(EqualityType::none, EqualityType::negated);
        EXPECT_NE(EqualityType::none, EqualityType::conjugated);
        EXPECT_NE(EqualityType::none, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::equal, EqualityType::negated);
        EXPECT_NE(EqualityType::equal, EqualityType::conjugated);
        EXPECT_NE(EqualityType::equal, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::negated, EqualityType::conjugated);
        EXPECT_NE(EqualityType::negated, EqualityType::neg_conj);
        EXPECT_NE(EqualityType::conjugated, EqualityType::neg_conj);
    }

    TEST(EqualityType, Or) {
        EXPECT_EQ(EqualityType::none | EqualityType::equal, EqualityType::equal);
        EXPECT_EQ(EqualityType::none | EqualityType::negated, EqualityType::negated);
        EXPECT_EQ(EqualityType::none | EqualityType::conjugated, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::none | EqualityType::neg_conj, EqualityType::neg_conj);

        EXPECT_EQ(EqualityType::equal | EqualityType::none, EqualityType::equal);
        EXPECT_EQ(EqualityType::negated | EqualityType::none, EqualityType::negated);
        EXPECT_EQ(EqualityType::conjugated | EqualityType::none, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::neg_conj | EqualityType::none, EqualityType::neg_conj);
    }

    TEST(EqualityType, And) {
        // Idempotent:
        EXPECT_EQ(EqualityType::none & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::equal, EqualityType::equal);
        EXPECT_EQ(EqualityType::negated & EqualityType::negated, EqualityType::negated);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::conjugated, EqualityType::conjugated);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::neg_conj, EqualityType::neg_conj);

        // Mutual exclusion:
        EXPECT_EQ(EqualityType::none & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::none & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::none, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::equal & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::equal, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::conjugated, EqualityType::none);
        EXPECT_EQ(EqualityType::negated & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::negated, EqualityType::none);
        EXPECT_EQ(EqualityType::conjugated & EqualityType::neg_conj, EqualityType::none);
        EXPECT_EQ(EqualityType::neg_conj & EqualityType::conjugated, EqualityType::none);
    }

    TEST(EqualityType, CreateFromPair) {
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{1}, SymbolExpression{2}}), EqualityType::equal);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{1}, SymbolExpression{-2}}), EqualityType::negated);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{1}, SymbolExpression{2, true}}), EqualityType::conjugated);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{1}, SymbolExpression{-2, true}}), EqualityType::neg_conj);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{-1}, SymbolExpression{2}}), EqualityType::negated);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{1, true}, SymbolExpression{2}}), EqualityType::conjugated);
        EXPECT_EQ(equality_type(SymbolPair{SymbolExpression{-1, true}, SymbolExpression{2}}), EqualityType::neg_conj);
    }

    TEST(EqualityType, Negate) {
        EXPECT_EQ(negate(EqualityType::equal), EqualityType::negated);
        EXPECT_EQ(negate(EqualityType::negated), EqualityType::equal);
        EXPECT_EQ(negate(EqualityType::conjugated), EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::neg_conj), EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::negated),
                  EqualityType::equal | EqualityType::negated);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::conjugated),
                  EqualityType::negated | EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::equal | EqualityType::neg_conj),
                  EqualityType::negated | EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::negated | EqualityType::conjugated),
                  EqualityType::equal | EqualityType::neg_conj);
        EXPECT_EQ(negate(EqualityType::negated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::conjugated);
        EXPECT_EQ(negate(EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::conjugated | EqualityType::neg_conj);

        EXPECT_EQ(negate(EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj);
    }

    TEST(EqualityType, Conjugate) {
        EXPECT_EQ(conjugate(EqualityType::equal), EqualityType::conjugated);
        EXPECT_EQ(conjugate(EqualityType::negated), EqualityType::neg_conj);
        EXPECT_EQ(conjugate(EqualityType::conjugated), EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::neg_conj), EqualityType::negated);

        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::negated),
                  EqualityType::conjugated | EqualityType::neg_conj);
        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::conjugated),
                  EqualityType::conjugated | EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::neg_conj),
                  EqualityType::conjugated | EqualityType::negated);
        EXPECT_EQ(conjugate(EqualityType::negated | EqualityType::conjugated),
                  EqualityType::neg_conj | EqualityType::equal);
        EXPECT_EQ(conjugate(EqualityType::negated | EqualityType::neg_conj),
                  EqualityType::neg_conj | EqualityType::negated);
        EXPECT_EQ(conjugate(EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated);

        EXPECT_EQ(conjugate(EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj),
                  EqualityType::equal | EqualityType::negated | EqualityType::conjugated | EqualityType::neg_conj);
    }

    TEST(EqualityType, Compose) {
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::equal), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::negated), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::conjugated), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::equal, EqualityType::neg_conj), EqualityType::neg_conj);

        EXPECT_EQ(compose(EqualityType::negated, EqualityType::equal), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::negated), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::conjugated), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::negated, EqualityType::neg_conj), EqualityType::conjugated);

        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::equal), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::negated), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::conjugated), EqualityType::equal);
        EXPECT_EQ(compose(EqualityType::conjugated, EqualityType::neg_conj), EqualityType::negated);

        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::equal), EqualityType::neg_conj);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::negated), EqualityType::conjugated);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::conjugated), EqualityType::negated);
        EXPECT_EQ(compose(EqualityType::neg_conj, EqualityType::neg_conj), EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureReal_Exxx) {
        EqualityType type = EqualityType::equal;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureReal_xNxx) {
        EqualityType type = EqualityType::negated;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }

    TEST(EqualityType, SimplifyPureReal_xxCx) {
        EqualityType type = EqualityType::conjugated;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureReal_xxxT) {
        EqualityType type = EqualityType::neg_conj;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }

    TEST(EqualityType, SimplifyPureReal_ExCx) {
        EqualityType type = EqualityType::equal | EqualityType::conjugated;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureReal_xNxT) {
        EqualityType type = EqualityType::negated | EqualityType::neg_conj;
        EqualityType simplified = simplifyPureReal(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }


    TEST(EqualityType, SimplifyPureImaginary_Exxx) {
        EqualityType type = EqualityType::equal;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureImaginary_xNxx) {
        EqualityType type = EqualityType::negated;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }

    TEST(EqualityType, SimplifyPureImaginary_xxCx) {
        EqualityType type = EqualityType::conjugated;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }

    TEST(EqualityType, SimplifyPureImaginary_xxxT) {
        EqualityType type = EqualityType::neg_conj;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureImaginary_ExxT) {
        EqualityType type = EqualityType::equal | EqualityType::neg_conj;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::equal);
    }

    TEST(EqualityType, SimplifyPureImaginary_xNCx) {
        EqualityType type = EqualityType::negated | EqualityType::conjugated;
        EqualityType simplified = simplifyPureImaginary(type);
        EXPECT_EQ(simplified, EqualityType::negated);
    }

    TEST(EqualityType, TestZero_Exxx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::equal);
        EXPECT_FALSE(real_is_zero);
        EXPECT_FALSE(im_is_zero);
    }

    TEST(EqualityType, TestZero_xNxx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::negated);
        EXPECT_FALSE(real_is_zero);
        EXPECT_FALSE(im_is_zero);
    }

    TEST(EqualityType, TestZero_xxCx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::conjugated);
        EXPECT_FALSE(real_is_zero);
        EXPECT_FALSE(im_is_zero);
    }

    TEST(EqualityType, TestZero_xxxT) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::neg_conj);
        EXPECT_FALSE(real_is_zero);
        EXPECT_FALSE(im_is_zero);
    }


    TEST(EqualityType, TestZero_ENxx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::equal | EqualityType::negated);
        EXPECT_TRUE(real_is_zero) << "Should be zero";
        EXPECT_TRUE(im_is_zero) << "Should be zero";
    }

    TEST(EqualityType, TestZero_ExCx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::equal | EqualityType::conjugated);
        EXPECT_FALSE(real_is_zero) << "Should be real number";
        EXPECT_TRUE(im_is_zero) << "Should be real number";
    }

    TEST(EqualityType, TestZero_ExxT) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::equal | EqualityType::neg_conj);
        EXPECT_TRUE(real_is_zero) << "Should be imaginary number";
        EXPECT_FALSE(im_is_zero) << "Should be imaginary number";
    }

    TEST(EqualityType, TestZero_xNCx) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::negated | EqualityType::conjugated);
        EXPECT_TRUE(real_is_zero) << "Should be imaginary number";
        EXPECT_FALSE(im_is_zero) << "Should be imaginary number";
    }

    TEST(EqualityType, TestZero_xNxT) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::negated | EqualityType::neg_conj);
        EXPECT_FALSE(real_is_zero) << "Should be real number";
        EXPECT_TRUE(im_is_zero) << "Should be real number";
    }

    TEST(EqualityType, TestZero_xxCT) {
        auto [real_is_zero, im_is_zero] = implies_zero(EqualityType::conjugated | EqualityType::neg_conj);
        EXPECT_TRUE(real_is_zero) << "Should be zero";
        EXPECT_TRUE(im_is_zero) << "Should be zero";
    }

    TEST(EqualityType, TestReflexiveZero_Exxx) {
        auto [real_is_zero, im_is_zero] = reflexive_implies_zero(EqualityType::equal);
        EXPECT_FALSE(real_is_zero) << "Should be unconstrained";
        EXPECT_FALSE(im_is_zero) << "Should be unconstrained";
    }

    TEST(EqualityType, TestReflexiveZero_xNxx) {
        auto [real_is_zero, im_is_zero] = reflexive_implies_zero(EqualityType::negated);
        EXPECT_TRUE(real_is_zero) << "Should be zero";
        EXPECT_TRUE(im_is_zero) << "Should be zero";
    }

    TEST(EqualityType, TestReflexiveZero_xxCx) {
        auto [real_is_zero, im_is_zero] = reflexive_implies_zero(EqualityType::conjugated);
        EXPECT_FALSE(real_is_zero) << "Should be real number";
        EXPECT_TRUE(im_is_zero) << "Should be real number";
    }

    TEST(EqualityType, TestReflexiveZero_xxxT) {
        auto [real_is_zero, im_is_zero] = reflexive_implies_zero(EqualityType::neg_conj);
        EXPECT_TRUE(real_is_zero) << "Should be imaginary number";
        EXPECT_FALSE(im_is_zero) << "Should be imaginary number";
    }


}