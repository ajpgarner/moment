/**
 * monomial_substitution_rule_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/monomial_substitution_rule.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Operators_Algebraic_MonomialSubRule, Conjugate) {

        ShortlexHasher hasher{3};

        // BBA -> BA
        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, hasher},
                                     HashedSequence{{2, 1}, hasher}};
        EXPECT_FALSE(msr.negated());

        // ABB -> AB
        auto conj_msr = msr.conjugate(hasher);
        EXPECT_FALSE(conj_msr.negated());

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 1);
        EXPECT_EQ(conj_msr.RHS()[1], 2);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Conjugate_WithNegation) {

        ShortlexHasher hasher{3};

        // BBA -> BA
        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, hasher},
                                     HashedSequence{{2, 1}, hasher}, true};
        EXPECT_TRUE(msr.negated());

        // ABB -> AB
        auto conj_msr = msr.conjugate(hasher);
        EXPECT_TRUE(conj_msr.negated());

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 1);
        EXPECT_EQ(conj_msr.RHS()[1], 2);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Conjugate_WithZero) {

        ShortlexHasher hasher{3};

        // BBA -> BA
        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, hasher},
                                     HashedSequence{true}, true};
        EXPECT_TRUE(msr.RHS().zero());

        // ABB -> AB
        auto conj_msr = msr.conjugate(hasher);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 0);
        EXPECT_TRUE(conj_msr.RHS().zero());
    }

    TEST(Operators_Algebraic_MonomialSubRule, Match_BBAtoBA) {
        std::vector<oper_name_t> sampleStr{1, 2, 2, 1};

        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                                     HashedSequence{{2, 1}, ShortlexHasher{3}}};
        EXPECT_FALSE(msr.negated());
        ASSERT_EQ(msr.Delta(), -1);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 3);
        EXPECT_EQ(newStr[0], 1);
        EXPECT_EQ(newStr[1], 2);
        EXPECT_EQ(newStr[2], 1);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Match_BBAtoId_ABBA) {
        std::vector<oper_name_t> sampleStr{1, 2, 2, 1};

        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                                     HashedSequence{{}, ShortlexHasher{3}}};
        EXPECT_FALSE(msr.negated());
        ASSERT_EQ(msr.Delta(), -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 1);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Match_BBAtoId_BBAB) {
        std::vector<oper_name_t> sampleStr{2, 2, 1, 2};

        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                                     HashedSequence{{}, ShortlexHasher{3}}};

        EXPECT_FALSE(msr.negated());
        ASSERT_EQ(msr.Delta(), -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin());

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 2);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Match_BBAtoMinusBA) {
        std::vector<oper_name_t> sampleStr{1, 2, 2, 1};

        MonomialSubstitutionRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                                     HashedSequence{{2, 1}, ShortlexHasher{3}}, true};

        ASSERT_TRUE(msr.negated());
        ASSERT_EQ(msr.Delta(), -1);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 3);
        EXPECT_EQ(newStr[0], 1);
        EXPECT_EQ(newStr[1], 2);
        EXPECT_EQ(newStr[2], 1);
    }

    TEST(Operators_Algebraic_MonomialSubRule, Implies_BtoA_XBYtoXAY) {
        ShortlexHasher hasher{5};
        MonomialSubstitutionRule b_to_a{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        MonomialSubstitutionRule xby_to_xay{HashedSequence{{3, 2, 4}, hasher}, HashedSequence{{3, 1, 4}, hasher}};

        EXPECT_TRUE(b_to_a.implies(b_to_a));
        EXPECT_TRUE(b_to_a.implies(xby_to_xay));
        EXPECT_FALSE(xby_to_xay.implies(b_to_a));
        EXPECT_TRUE(xby_to_xay.implies(xby_to_xay));
    }

    TEST(Operators_Algebraic_MonomialSubRule, Implies_BBAtoA_XBBAYtoXAY) {
        ShortlexHasher hasher{5};
        MonomialSubstitutionRule bba_to_a{HashedSequence{{2, 2, 1}, hasher}, HashedSequence{{1}, hasher}};
        MonomialSubstitutionRule xbbay_to_xay{HashedSequence{{3, 2, 2, 1, 4}, hasher}, HashedSequence{{3, 1, 4}, hasher}};

        EXPECT_TRUE(bba_to_a.implies(bba_to_a));
        EXPECT_TRUE(bba_to_a.implies(xbbay_to_xay));
        EXPECT_FALSE(xbbay_to_xay.implies(bba_to_a));
        EXPECT_TRUE(xbbay_to_xay.implies(xbbay_to_xay));
    }

    TEST(Operators_Algebraic_MonomialSubRule, Implies_BtoA_DtoC) {
        ShortlexHasher hasher{5};
        MonomialSubstitutionRule b_to_a{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        MonomialSubstitutionRule d_to_c{HashedSequence{{4}, hasher}, HashedSequence{{3}, hasher}};

        EXPECT_TRUE(b_to_a.implies(b_to_a));
        EXPECT_FALSE(b_to_a.implies(d_to_c));
        EXPECT_FALSE(d_to_c.implies(b_to_a));
        EXPECT_TRUE(d_to_c.implies(d_to_c));
    }

    TEST(Operators_Algebraic_MonomialSubRule, Combine_ABtoA_BAtoB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        auto joint01_opt = msr[0].combine(msr[1], hasher);
        ASSERT_TRUE(joint01_opt.has_value());
        auto& joint01 = joint01_opt.value();

        ASSERT_EQ(joint01.LHS().size(), 2);
        EXPECT_EQ(joint01.LHS()[0], 0);
        EXPECT_EQ(joint01.LHS()[1], 1);

        ASSERT_EQ(joint01.RHS().size(), 2);
        EXPECT_EQ(joint01.RHS()[0], 0);
        EXPECT_EQ(joint01.RHS()[1], 0);

        auto joint10_opt = msr[1].combine(msr[0], hasher);
        ASSERT_TRUE(joint10_opt.has_value());
        auto& joint10 = joint10_opt.value();

        ASSERT_EQ(joint10.LHS().size(), 2);
        EXPECT_EQ(joint10.LHS()[0], 1);
        EXPECT_EQ(joint10.LHS()[1], 1);

        ASSERT_EQ(joint10.RHS().size(), 2);
        EXPECT_EQ(joint10.RHS()[0], 1);
        EXPECT_EQ(joint10.RHS()[1], 0);

    }

    TEST(Operators_Algebraic_MonomialSubRule, Combine_XYXYXYtoId_YYYtoId) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1, 0, 1, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{1, 1, 1}, hasher},
                         HashedSequence{{}, hasher});
        auto joint01_opt = msr[0].combine(msr[1], hasher);
        ASSERT_TRUE(joint01_opt.has_value());
        auto& joint01 = joint01_opt.value();

        ASSERT_EQ(joint01.LHS().size(), 5);
        EXPECT_EQ(joint01.LHS()[0], 0);
        EXPECT_EQ(joint01.LHS()[1], 1);
        EXPECT_EQ(joint01.LHS()[2], 0);
        EXPECT_EQ(joint01.LHS()[3], 1);
        EXPECT_EQ(joint01.LHS()[4], 0);

        ASSERT_EQ(joint01.RHS().size(), 2);
        EXPECT_EQ(joint01.RHS()[0], 1);
        EXPECT_EQ(joint01.RHS()[1], 1);

        auto joint10_opt = msr[1].combine(msr[0], hasher);
        ASSERT_FALSE(joint10_opt.has_value());


    }


    TEST(Operators_Algebraic_MonomialSubRule, Combine_ABtoA_BAtoMinusB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher}, true);
        auto joint01_opt = msr[0].combine(msr[1], hasher);
        ASSERT_TRUE(joint01_opt.has_value());
        auto& joint01 = joint01_opt.value();

        ASSERT_EQ(joint01.LHS().size(), 2);
        EXPECT_EQ(joint01.LHS()[0], 0);
        EXPECT_EQ(joint01.LHS()[1], 1);

        ASSERT_EQ(joint01.RHS().size(), 2);
        EXPECT_EQ(joint01.RHS()[0], 0);
        EXPECT_EQ(joint01.RHS()[1], 0);

        EXPECT_TRUE(joint01.negated());

        auto joint10_opt = msr[1].combine(msr[0], hasher);
        ASSERT_TRUE(joint10_opt.has_value());
        auto& joint10 = joint10_opt.value();

        ASSERT_EQ(joint10.LHS().size(), 2);
        EXPECT_EQ(joint10.LHS()[0], 1);
        EXPECT_EQ(joint10.LHS()[1], 1);

        ASSERT_EQ(joint10.RHS().size(), 2);
        EXPECT_EQ(joint10.RHS()[0], 1);
        EXPECT_EQ(joint10.RHS()[1], 0);

        EXPECT_TRUE(joint10.negated());

    }

}