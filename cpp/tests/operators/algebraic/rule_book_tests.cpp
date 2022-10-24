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
    }


    TEST(RuleBook, Reduce_String) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr};

        auto simplified_string = rules.reduce(
                HashedSequence{{0, 1}, hasher}
        );

        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_EQ(simplified_string[0], 0);
    }

     TEST(RuleBook, Reduce_StringRecursive) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        RuleBook rules{hasher, msr};

        auto simplified_string = rules.reduce(
                HashedSequence{{0, 1, 1, 1}, hasher}
        );

        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_EQ(simplified_string[0], 0);
    }


    TEST(RuleBook, Reduce_Rule) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr};

        auto simplified_rule = rules.reduce(
               MonomialSubstitutionRule{HashedSequence{{0, 1}, hasher}, HashedSequence{{0, 0}, hasher}}
                );

        ASSERT_EQ(simplified_rule.LHS().size(), 2); // 00
        EXPECT_EQ(simplified_rule.LHS()[0], 0);
        EXPECT_EQ(simplified_rule.LHS()[1], 0);

        ASSERT_EQ(simplified_rule.RHS().size(), 1); // 01 -> 0
        EXPECT_EQ(simplified_rule.RHS()[0], 0);

    }

    TEST(RuleBook, ReduceRuleset_AACtoAAB_CtoB) {
        ShortlexHasher hasher{3};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 2}, hasher},
                         HashedSequence{{0, 0, 1}, hasher});
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr};

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
        RuleBook rules{hasher, msr};

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

    TEST(RuleBook, IterativelyDeduce_ABtoA_BAtoB) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        RuleBook rules{hasher, msr};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.try_new_reduction());
        ASSERT_EQ(rules.rules().size(), 3); // Should add 00 -> 0

        ASSERT_TRUE(rules.try_new_reduction());
        ASSERT_EQ(rules.rules().size(), 4); // Should add 11 -> 1

        ASSERT_FALSE(rules.try_new_reduction()); // No further confluences

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}), (HashedSequence{{0}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 1}, hasher}), (HashedSequence{{0}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0}, hasher}), (HashedSequence{{1}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1}, hasher}), (HashedSequence{{1}, hasher}));

        EXPECT_TRUE(rules.is_complete());
    }

    TEST(RuleBook, IterativelyDeduce_AAAtoI_BBBtoI_ABABABtoI) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 0}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{1, 1, 1}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{0, 1, 0, 1, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        RuleBook rules{hasher, msr};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.complete(20));
        EXPECT_EQ(rules.rules().size(), 4);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 0}, hasher}), (HashedSequence{{}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 1}, hasher}), (HashedSequence{{}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 1, 0}, hasher}), (HashedSequence{{0, 0, 1, 1}, hasher}));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 0, 0}, hasher}), (HashedSequence{{0, 1, 0, 1}, hasher}));

        EXPECT_TRUE(rules.is_complete());
    }
    
}