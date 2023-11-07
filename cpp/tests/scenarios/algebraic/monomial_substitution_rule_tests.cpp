/**
 * monomial_substitution_rule_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/operator_rule.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Scenarios_Algebraic_MonomialSubRule, OrientSign) {

        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;

        // -BBA -> BA
        OperatorRule msr{HashedSequence{{2, 2, 1}, hasher, SequenceSignType::Negative},
                         HashedSequence{{2, 1}, hasher, SequenceSignType::Positive}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Negative);
        EXPECT_EQ(msr.LHS(), HashedSequence({2, 2, 1}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(msr.RHS(), HashedSequence({2, 1}, hasher, SequenceSignType::Negative));
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_SelfAdjoint) {

        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;

        // BBA -> BA
        OperatorRule msr{HashedSequence{{2, 2, 1}, hasher},
                         HashedSequence{{2, 1}, hasher}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);

        // ABB -> AB
        auto conj_msr = msr.conjugate(apc);
        EXPECT_EQ(conj_msr.rule_sign(), SequenceSignType::Positive);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 1);
        EXPECT_EQ(conj_msr.RHS()[1], 2);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_Bunched) {

        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;

        // BBA -> BA
        OperatorRule msr{HashedSequence{{2, 2, 1}, hasher},
                         HashedSequence{{2, 1}, hasher}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);

        // A*B*B* -> A*B*
        auto conj_msr = msr.conjugate(apc);
        EXPECT_EQ(conj_msr.rule_sign(), SequenceSignType::Positive);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 4);
        EXPECT_EQ(conj_msr.LHS()[1], 5);
        EXPECT_EQ(conj_msr.LHS()[2], 5);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 4);
        EXPECT_EQ(conj_msr.RHS()[1], 5);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_Interleaved) {

        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Interleaved};
        const ShortlexHasher& hasher = apc.hasher;

        // BBA -> BA
        OperatorRule msr{HashedSequence{{4, 4, 2}, hasher},
                         HashedSequence{{4, 2}, hasher}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);

        // A*B*B* -> A*B*
        auto conj_msr = msr.conjugate(apc);
        EXPECT_EQ(conj_msr.rule_sign(), SequenceSignType::Positive);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 3);
        EXPECT_EQ(conj_msr.LHS()[1], 5);
        EXPECT_EQ(conj_msr.LHS()[2], 5);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 3);
        EXPECT_EQ(conj_msr.RHS()[1], 5);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_WithNegation) {
        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;

        // BBA -> BA
        OperatorRule msr{HashedSequence{{2, 2, 1}, hasher},
                         HashedSequence{{2, 1}, hasher, SequenceSignType::Negative}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Negative);

        // ABB -> AB
        auto conj_msr = msr.conjugate(apc);
        EXPECT_EQ(conj_msr.rule_sign(), SequenceSignType::Negative);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 2);
        EXPECT_EQ(conj_msr.RHS()[0], 1);
        EXPECT_EQ(conj_msr.RHS()[1], 2);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_AntiCommutator) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        const ShortlexHasher& hasher = apc.hasher;

        // BA -> -BA
        OperatorRule msr{HashedSequence{{1, 0}, hasher},
                         HashedSequence{{0, 1}, hasher, SequenceSignType::Negative}}; // ba -> -ab
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Negative);

        // BB -> -AB also
        auto conj_msr = msr.conjugate(apc);
        EXPECT_EQ(msr.LHS(), conj_msr.LHS());
        EXPECT_EQ(msr.RHS(), conj_msr.RHS());
        EXPECT_FALSE(conj_msr.implies_zero());
        EXPECT_FALSE(conj_msr.trivial());
        EXPECT_EQ(conj_msr.rule_sign(), SequenceSignType::Negative);

    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Conjugate_WithZero) {
        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;

        // BBA -> BA
        OperatorRule msr{HashedSequence{{2, 2, 1}, hasher},
                         HashedSequence{true}};
        EXPECT_TRUE(msr.RHS().zero());

        // ABB -> AB
        auto conj_msr = msr.conjugate(apc);

        ASSERT_EQ(conj_msr.LHS().size(), 3);
        EXPECT_EQ(conj_msr.LHS()[0], 1);
        EXPECT_EQ(conj_msr.LHS()[1], 2);
        EXPECT_EQ(conj_msr.LHS()[2], 2);

        ASSERT_EQ(conj_msr.RHS().size(), 0);
        EXPECT_TRUE(conj_msr.RHS().zero());
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Match_BBAtoBA) {
        sequence_storage_t sampleStr{1, 2, 2, 1};

        OperatorRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                         HashedSequence{{2, 1}, ShortlexHasher{3}}};
        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);
        ASSERT_EQ(msr.Delta(), -1);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 3);
        EXPECT_EQ(newStr[0], 1);
        EXPECT_EQ(newStr[1], 2);
        EXPECT_EQ(newStr[2], 1);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Match_BBAtoId_ABBA) {
        sequence_storage_t sampleStr{1, 2, 2, 1};

        OperatorRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                         HashedSequence{{}, ShortlexHasher{3}}};

        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);
        ASSERT_EQ(msr.Delta(), -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 1);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Match_BBAtoId_BBAB) {
        sequence_storage_t sampleStr{2, 2, 1, 2};

        OperatorRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                         HashedSequence{{}, ShortlexHasher{3}}};

        EXPECT_EQ(msr.rule_sign(), SequenceSignType::Positive);
        ASSERT_EQ(msr.Delta(), -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin());

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 2);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Match_BBAtoMinusBA) {
        sequence_storage_t sampleStr{1, 2, 2, 1};

        OperatorRule msr{HashedSequence{{2, 2, 1}, ShortlexHasher{3}},
                         HashedSequence{{2, 1}, ShortlexHasher{3}, SequenceSignType::Negative}};

        ASSERT_EQ(msr.rule_sign(), SequenceSignType::Negative);
        ASSERT_EQ(msr.Delta(), -1);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 3);
        EXPECT_EQ(newStr[0], 1);
        EXPECT_EQ(newStr[1], 2);
        EXPECT_EQ(newStr[2], 1);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Implies_BtoA_XBYtoXAY) {
        ShortlexHasher hasher{5};
        OperatorRule b_to_a{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        OperatorRule xby_to_xay{HashedSequence{{3, 2, 4}, hasher}, HashedSequence{{3, 1, 4}, hasher}};

        EXPECT_TRUE(b_to_a.implies(b_to_a));
        EXPECT_TRUE(b_to_a.implies(xby_to_xay));
        EXPECT_FALSE(xby_to_xay.implies(b_to_a));
        EXPECT_TRUE(xby_to_xay.implies(xby_to_xay));
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Implies_BBAtoA_XBBAYtoXAY) {
        ShortlexHasher hasher{5};
        OperatorRule bba_to_a{HashedSequence{{2, 2, 1}, hasher}, HashedSequence{{1}, hasher}};
        OperatorRule xbbay_to_xay{HashedSequence{{3, 2, 2, 1, 4}, hasher}, HashedSequence{{3, 1, 4}, hasher}};

        EXPECT_TRUE(bba_to_a.implies(bba_to_a));
        EXPECT_TRUE(bba_to_a.implies(xbbay_to_xay));
        EXPECT_FALSE(xbbay_to_xay.implies(bba_to_a));
        EXPECT_TRUE(xbbay_to_xay.implies(xbbay_to_xay));
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Implies_BtoA_DtoC) {
        ShortlexHasher hasher{5};
        OperatorRule b_to_a{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        OperatorRule d_to_c{HashedSequence{{4}, hasher}, HashedSequence{{3}, hasher}};

        EXPECT_TRUE(b_to_a.implies(b_to_a));
        EXPECT_FALSE(b_to_a.implies(d_to_c));
        EXPECT_FALSE(d_to_c.implies(b_to_a));
        EXPECT_TRUE(d_to_c.implies(d_to_c));
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Combine_ABtoA_BAtoB) {

        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;

        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        auto joint01_opt = msr[0].combine(msr[1], apc);
        ASSERT_TRUE(joint01_opt.has_value());
        auto& joint01 = joint01_opt.value();

        ASSERT_EQ(joint01.LHS().size(), 2);
        EXPECT_EQ(joint01.LHS()[0], 0);
        EXPECT_EQ(joint01.LHS()[1], 1);

        ASSERT_EQ(joint01.RHS().size(), 2);
        EXPECT_EQ(joint01.RHS()[0], 0);
        EXPECT_EQ(joint01.RHS()[1], 0);

        auto joint10_opt = msr[1].combine(msr[0], apc);
        ASSERT_TRUE(joint10_opt.has_value());
        auto& joint10 = joint10_opt.value();

        ASSERT_EQ(joint10.LHS().size(), 2);
        EXPECT_EQ(joint10.LHS()[0], 1);
        EXPECT_EQ(joint10.LHS()[1], 1);

        ASSERT_EQ(joint10.RHS().size(), 2);
        EXPECT_EQ(joint10.RHS()[0], 1);
        EXPECT_EQ(joint10.RHS()[1], 0);
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Combine_XYXYXYtoId_YYYtoId) {
        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;

        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1, 0, 1, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{1, 1, 1}, hasher},
                         HashedSequence{{}, hasher});
        auto joint01_opt = msr[0].combine(msr[1], apc);
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

        auto joint10_opt = msr[1].combine(msr[0], apc);
        ASSERT_FALSE(joint10_opt.has_value());
    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Combine_ABtoA_BAtoMinusB) {

        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;

        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher, SequenceSignType::Negative});
        auto joint01_opt = msr[0].combine(msr[1], apc);
        ASSERT_TRUE(joint01_opt.has_value());
        auto& joint01 = joint01_opt.value();

        ASSERT_EQ(joint01.LHS().size(), 2);
        EXPECT_EQ(joint01.LHS()[0], 0);
        EXPECT_EQ(joint01.LHS()[1], 1);

        ASSERT_EQ(joint01.RHS().size(), 2);
        EXPECT_EQ(joint01.RHS()[0], 0);
        EXPECT_EQ(joint01.RHS()[1], 0);

        EXPECT_EQ(joint01.rule_sign(), SequenceSignType::Negative);

        auto joint10_opt = msr[1].combine(msr[0], apc);
        ASSERT_TRUE(joint10_opt.has_value());
        auto& joint10 = joint10_opt.value();

        ASSERT_EQ(joint10.LHS().size(), 2);
        EXPECT_EQ(joint10.LHS()[0], 1);
        EXPECT_EQ(joint10.LHS()[1], 1);

        ASSERT_EQ(joint10.RHS().size(), 2);
        EXPECT_EQ(joint10.RHS()[0], 1);
        EXPECT_EQ(joint10.RHS()[1], 0);
        EXPECT_EQ(joint10.rule_sign(), SequenceSignType::Negative);

    }

    TEST(Scenarios_Algebraic_MonomialSubRule, Combine_ImplyZero) {

        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;

        const OperatorRule ruleA{HashedSequence{{1, 0}, hasher},
                                 HashedSequence{{0, 1}, hasher, SequenceSignType::Negative}};
        const OperatorRule ruleB{HashedSequence{{0, 0}, hasher},
                                 HashedSequence{{0}, hasher}};
        auto combined_rule = ruleA.combine(ruleB, apc);
        ASSERT_TRUE(combined_rule.has_value());
        EXPECT_EQ(combined_rule->LHS(), HashedSequence({0, 1, 0}, hasher));
        EXPECT_EQ(combined_rule->RHS(), HashedSequence({1, 0}, hasher, SequenceSignType::Negative));

    }

}