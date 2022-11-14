/**
 * rule_book_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/algebraic/algebraic_context.h"
#include "operators/algebraic/rule_book.h"

namespace NPATK::Tests {

    TEST(RuleBook, Empty) {
        ShortlexHasher slh{0};
        RuleBook rules{slh};
        EXPECT_EQ(rules.size(), 0);
        EXPECT_TRUE(rules.rules().empty());
    }

    TEST(RuleBook, AddRule_ToEmpty) {
        ShortlexHasher hasher{2};
        RuleBook rules{hasher};
        EXPECT_EQ(rules.size(), 0);
        MonomialSubstitutionRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        ASSERT_EQ(rules.size(), 1);

        auto theRule = rules.rules().find(hasher.hash({0, 1}));
        ASSERT_NE(theRule, rules.rules().cend());
        EXPECT_EQ(theRule->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRule->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRule->second.negated());
    }

    TEST(RuleBook, AddRule_ToNonEmpty) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 1);

        MonomialSubstitutionRule msr{HashedSequence{{0, 2}, hasher}, HashedSequence{{1}, hasher}, true};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher.hash({0, 1}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleA->second.negated());

        auto theRuleB = rules.rules().find(hasher.hash({0, 2}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({0, 2}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({1}, hasher));
        EXPECT_TRUE(theRuleB->second.negated());
    }

    TEST(RuleBook, AddRule_Redundant) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 1);

        MonomialSubstitutionRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 0);
        EXPECT_EQ(rules.size(), 1);

        auto theRuleA = rules.rules().find(hasher.hash({0, 1}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleA->second.negated());
    }

    TEST(RuleBook, AddRule_ImpliesZero) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 1);

        MonomialSubstitutionRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher}, true};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher.hash({0, 1}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence(true));
        EXPECT_FALSE(theRuleA->second.negated());

        auto theRuleB = rules.rules().find(hasher.hash({0}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence(true));
        EXPECT_FALSE(theRuleB->second.negated());
    }

    TEST(RuleBook, AddRule_CtoB_CtoA) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 1);

        MonomialSubstitutionRule msr{HashedSequence{{2}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher.hash({2}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleA->second.negated());

        auto theRuleB = rules.rules().find(hasher.hash({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleB->second.negated());
    }

    TEST(RuleBook, AddRule_CtoA_CtoB) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 1);

        MonomialSubstitutionRule msr{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher.hash({2}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleA->second.negated());

        auto theRuleB = rules.rules().find(hasher.hash({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleB->second.negated());
    }

    TEST(RuleBook, AddRule_Cascade) {
        ShortlexHasher hasher{4};
        std::vector<MonomialSubstitutionRule> msr_list;
        msr_list.emplace_back(HashedSequence{{3}, hasher},
                         HashedSequence{{2}, hasher}); // D -> C
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{0}, hasher}); // C -> A
        RuleBook rules{hasher, msr_list, false};
        EXPECT_EQ(rules.size(), 2);

        MonomialSubstitutionRule msr{HashedSequence{{3}, hasher}, HashedSequence{{1}, hasher}}; // D -> B
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 3);

        auto theRuleD = rules.rules().find(hasher.hash({3}));
        ASSERT_NE(theRuleD, rules.rules().cend());
        EXPECT_EQ(theRuleD->second.LHS(), HashedSequence({3}, hasher));
        EXPECT_EQ(theRuleD->second.RHS(), HashedSequence({1}, hasher));
        EXPECT_FALSE(theRuleD->second.negated());

        auto theRuleC = rules.rules().find(hasher.hash({2}));
        ASSERT_NE(theRuleC, rules.rules().cend());
        EXPECT_EQ(theRuleC->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleC->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleC->second.negated());

        auto theRuleB = rules.rules().find(hasher.hash({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_FALSE(theRuleB->second.negated());
    }

    TEST(RuleBook, Reduce_String) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr, false};

        auto [simplified_string, neg] = rules.reduce(
                HashedSequence{{0, 1}, hasher}
        );

        EXPECT_FALSE(neg);
        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_EQ(simplified_string[0], 0);
    }

     TEST(RuleBook, Reduce_StringRecursive) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr, false};

        auto [simplified_string, neg] = rules.reduce(
                HashedSequence{{0, 1, 1, 1}, hasher}
        );

        EXPECT_FALSE(neg);
        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_EQ(simplified_string[0], 0);
    }

    TEST(RuleBook, Reduce_ABToZero_AB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        RuleBook rules{hasher, msr, false};

        auto [simplified_string, neg] = rules.reduce(
                HashedSequence{{0, 1}, hasher}
        );

        EXPECT_FALSE(neg);
        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_TRUE(simplified_string.zero());
    }

    TEST(RuleBook, Reduce_ABToZero_ABBB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        RuleBook rules{hasher, msr, false};

        auto [simplified_string, neg] = rules.reduce(
                HashedSequence{{0, 1, 1, 1}, hasher}
        );

        EXPECT_FALSE(neg);
        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_TRUE(simplified_string.zero());
    }

    TEST(RuleBook, Reduce_ABToZero_BAB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        RuleBook rules{hasher, msr, false};

        auto [simplified_string, neg] = rules.reduce(
                HashedSequence{{1, 0, 1}, hasher}
        );

        EXPECT_FALSE(neg);
        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_TRUE(simplified_string.zero());
    }

    TEST(RuleBook, Reduce_Rule) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr, false};

        auto simplified_rule = rules.reduce(
               MonomialSubstitutionRule{HashedSequence{{0, 1}, hasher}, HashedSequence{{0, 0}, hasher}}
                );

        ASSERT_EQ(simplified_rule.LHS().size(), 2); // 00
        EXPECT_EQ(simplified_rule.LHS()[0], 0);
        EXPECT_EQ(simplified_rule.LHS()[1], 0);

        ASSERT_EQ(simplified_rule.RHS().size(), 1); // 01 -> 0
        EXPECT_EQ(simplified_rule.RHS()[0], 0);

    }

    TEST(RuleBook, Reduce_RuleToZero) {
        ShortlexHasher hasher{4};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher}); // 2 = 1
        msr.emplace_back(HashedSequence{{3}, hasher},
                         HashedSequence{{1}, hasher}, true); // 3 = -1
        RuleBook rules{hasher, msr, false};

        auto simplified_rule = rules.reduce(
               MonomialSubstitutionRule{HashedSequence{{3}, hasher}, HashedSequence{{2}, hasher}}
                );

        // Rule reduces to 1 = -1 => 1 = [null]
        ASSERT_EQ(simplified_rule.LHS().size(), 1);
        EXPECT_EQ(simplified_rule.LHS()[0], 1);

        ASSERT_EQ(simplified_rule.RHS().size(), 0);
        EXPECT_TRUE(simplified_rule.RHS().zero());
    }

    TEST(RuleBook, ReduceRuleset_AACtoAAB_CtoB) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 2}, hasher},
                         HashedSequence{{0, 0, 1}, hasher});
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr, false};

        size_t number_reduced = rules.reduce_ruleset();
        EXPECT_EQ(number_reduced, 1); // should have removed 002->001

        const auto& rule_map = rules.rules();
        auto rule_map_iter = rule_map.cbegin();
        ASSERT_NE(rule_map_iter, rule_map.cend());
        EXPECT_EQ(rule_map_iter->first, hasher({2}));

        const auto& rule = rule_map_iter->second;
        ASSERT_EQ(rule.LHS().size(), 1);
        EXPECT_EQ(rule.LHS()[0], 2);
        ASSERT_EQ(rule.RHS().size(), 1);
        EXPECT_EQ(rule.RHS()[0], 1);

        ++rule_map_iter;
        ASSERT_EQ(rule_map_iter, rule_map.cend());
    }

    TEST(RuleBook, ReduceRuleset_CtoB_BtoA) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        msr.emplace_back(HashedSequence{{1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr, false};

        size_t number_reduced = rules.reduce_ruleset();
        EXPECT_EQ(number_reduced, 1); // should have altered 2->1 to 2->0

        const auto& rule_map = rules.rules();
        auto rule_map_iter = rule_map.cbegin();
        ASSERT_NE(rule_map_iter, rule_map.cend());
        EXPECT_EQ(rule_map_iter->first, hasher({1}));

        const auto& rule1 = rule_map_iter->second;
        ASSERT_EQ(rule1.LHS().size(), 1);
        EXPECT_EQ(rule1.LHS()[0], 1);
        ASSERT_EQ(rule1.RHS().size(), 1);
        EXPECT_EQ(rule1.RHS()[0], 0);

        ++rule_map_iter;
        ASSERT_NE(rule_map_iter, rule_map.cend());
        EXPECT_EQ(rule_map_iter->first, hasher({2}));

        const auto& rule2 = rule_map_iter->second;
        ASSERT_EQ(rule2.LHS().size(), 1);
        EXPECT_EQ(rule2.LHS()[0], 2);
        ASSERT_EQ(rule2.RHS().size(), 1);
        EXPECT_EQ(rule2.RHS()[0], 0);

        ++rule_map_iter;
        ASSERT_EQ(rule_map_iter, rule_map.cend());
    }

    TEST(RuleBook, AddConjugateRule) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        RuleBook rules{hasher, msr, true};

        ASSERT_EQ(rules.rules().size(), 1);
        EXPECT_TRUE(rules.try_conjugation(rules.rules().begin()->second));
        EXPECT_EQ(rules.rules().size(), 2);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 1}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 0}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
    }

    TEST(RuleBook, ConjugateRuleset) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        RuleBook rules{hasher, msr, true};

        ASSERT_EQ(rules.rules().size(), 1);

        EXPECT_EQ(rules.conjugate_ruleset(), 1);
        EXPECT_EQ(rules.rules().size(), 2);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 1}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 0}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
    }

    TEST(RuleBook, Complete_ABtoA_BAtoB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr, false};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.try_new_combination());
        ASSERT_EQ(rules.rules().size(), 3); // Should add 00 -> 0

        ASSERT_TRUE(rules.try_new_combination());
        ASSERT_EQ(rules.rules().size(), 4); // Should add 11 -> 1

        ASSERT_FALSE(rules.try_new_combination()); // No further confluences

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 1}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0}, hasher}), std::make_pair(HashedSequence{{1}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1}, hasher}), std::make_pair(HashedSequence{{1}, hasher}, false));

        EXPECT_TRUE(rules.is_complete());
    }

    TEST(RuleBook, Complete_AAAtoI_BBBtoI_ABABABtoI) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 0}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{1, 1, 1}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{0, 1, 0, 1, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        RuleBook rules{hasher, msr, false};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.complete(20));
        EXPECT_EQ(rules.rules().size(), 4);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 0}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 1}, hasher}), std::make_pair(HashedSequence{{}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 1, 0}, hasher}),
                  std::make_pair(HashedSequence{{0, 0, 1, 1}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 0, 0}, hasher}),
                  std::make_pair(HashedSequence{{0, 1, 0, 1}, hasher}, false));

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(RuleBook, Complete_ABtoA_BAtoMinusB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher}, true);
        RuleBook rules{hasher, msr, false};

        EXPECT_FALSE(rules.is_complete());
        ASSERT_TRUE(rules.complete(10));

        // aa = -a; ab = a; ba = -b; bb = b
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, true));
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 1}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0}, hasher}), std::make_pair(HashedSequence{{1}, hasher}, true));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1}, hasher}), std::make_pair(HashedSequence{{1}, hasher}, false));

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(RuleBook, HermitianComplete_ABtoA_BAtoB_Hermitian) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr, true};

        EXPECT_FALSE(rules.is_complete());

        rules.complete(10);
        ASSERT_EQ(rules.rules().size(), 2); // Should end up with 1 -> 0 and 00 -> 0.
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));

        EXPECT_TRUE(rules.is_complete());
    }

    TEST(RuleBook, HermitianComplete_ABtoA_BCtoB_CAtoC) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 2}, hasher},
                         HashedSequence{{1}, hasher});
        msr.emplace_back(HashedSequence{{2, 0}, hasher},
                         HashedSequence{{2}, hasher});
        RuleBook rules{hasher, msr, true};

        EXPECT_FALSE(rules.is_complete());

        rules.complete(10);
        ASSERT_EQ(rules.rules().size(), 3); // Should end up with 1 -> 0, 2 -> 0 and 00 -> 0.

        auto rule_iter = rules.rules().cbegin();
        ASSERT_NE(rule_iter, rules.rules().cend());
        EXPECT_EQ(rule_iter->first, hasher({1}));
        ++rule_iter;

        ASSERT_NE(rule_iter, rules.rules().cend());
        EXPECT_EQ(rule_iter->first, hasher({2}));
        ++rule_iter;

        ASSERT_NE(rule_iter, rules.rules().cend());
        EXPECT_EQ(rule_iter->first, hasher({0, 0}));
        ++rule_iter;

        ASSERT_EQ(rule_iter, rules.rules().cend());

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{1}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));
        EXPECT_EQ(rules.reduce(HashedSequence{{2}, hasher}), std::make_pair(HashedSequence{{0}, hasher}, false));

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(RuleBook, GenerateCommutators) {
        ShortlexHasher hasher{3};
        auto comVec = RuleBook::commutator_rules(hasher, 3);
        ASSERT_EQ(comVec.size(), 3);

        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(comVec[i].LHS().size(), 2) << i;
            ASSERT_EQ(comVec[i].RHS().size(), 2) << i;
        }

        EXPECT_EQ(comVec[0].LHS(),HashedSequence({2, 1}, hasher));
        EXPECT_EQ(comVec[0].RHS(),HashedSequence({1, 2}, hasher));

        EXPECT_EQ(comVec[1].LHS(),HashedSequence({2, 0}, hasher));
        EXPECT_EQ(comVec[1].RHS(),HashedSequence({0, 2}, hasher));

        EXPECT_EQ(comVec[2].LHS(),HashedSequence({1, 0}, hasher));
        EXPECT_EQ(comVec[2].RHS(),HashedSequence({0, 1}, hasher));

    }


}