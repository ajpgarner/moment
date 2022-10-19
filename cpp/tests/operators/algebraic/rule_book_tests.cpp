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

    TEST(RuleBook, MergeOne_ABtoI_BAtoI) {
        ShortlexHasher hasher{2};
        std::vector<MonomialSubstitutionRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});

        RuleBook rules{hasher, msr};

        auto overlap = RuleBook::rule_overlap_lhs(msr[0], msr[1]);
        ASSERT_EQ(overlap, 1);

        auto joint_seq = RuleBook::concat_merge_lhs(msr[0], msr[1], overlap);
        ASSERT_EQ(joint_seq.size(), 3);
        EXPECT_EQ(joint_seq[0], 0);
        EXPECT_EQ(joint_seq[1], 1);
        EXPECT_EQ(joint_seq[0], 0);

        auto joint_rule = rules.combine_rules(msr[0], msr[1], overlap);
        ASSERT_EQ(joint_rule.LHS().size(), 2);
        EXPECT_EQ(joint_rule.LHS().raw()[0], 0);
        EXPECT_EQ(joint_rule.LHS().raw()[1], 0);

        ASSERT_EQ(joint_rule.RHS().size(), 1);
        EXPECT_EQ(joint_rule.RHS().raw()[0], 0);



    }
}