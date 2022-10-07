/**
 * monomial_substitution_rule_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/algebraic/algebraic_context.h"
#include "operators/algebraic/monomial_substitution_rule.h"
#include "operators/algebraic/raw_sequence_book.h"


namespace NPATK::Tests {

    TEST(MonomialSubRule, MatchABinABAB) {
        std::vector<oper_name_t> sampleStr{1, 2, 1, 2};

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{3, 4}};

        EXPECT_TRUE(msr.matches(sampleStr.begin(), sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 1, sampleStr.end()));
        EXPECT_TRUE(msr.matches(sampleStr.begin() + 2, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 3, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.end(), sampleStr.end()));

        auto matchA = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        EXPECT_EQ(matchA, sampleStr.begin());
        auto matchB = msr.matches_anywhere(matchA + 1, sampleStr.end());
        EXPECT_EQ(matchB, sampleStr.begin() + 2);
        auto matchC = msr.matches_anywhere(matchB + 1, sampleStr.end());
        EXPECT_EQ(matchC, sampleStr.end());
    }

    TEST(MonomialSubRule, MatchABinBABA) {
        std::vector<oper_name_t> sampleStr{2, 1, 2, 1};

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{3, 4}};

        EXPECT_FALSE(msr.matches(sampleStr.begin(), sampleStr.end()));
        EXPECT_TRUE(msr.matches(sampleStr.begin() + 1, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 2, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 3, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.end(), sampleStr.end()));

        auto matchA = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        EXPECT_EQ(matchA, sampleStr.begin() + 1);
        auto matchB = msr.matches_anywhere(matchA + 1, sampleStr.end());
        EXPECT_EQ(matchB, sampleStr.end());
    }

    TEST(MonomialSubRule, MatchBBAtoBA) {
        std::vector<oper_name_t> sampleStr{1, 2, 2, 1};

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{2, 2, 1},
                                     std::vector<oper_name_t>{2, 1}};

        ASSERT_EQ(msr.Delta, -1);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 3);
        EXPECT_EQ(newStr[0], 1);
        EXPECT_EQ(newStr[1], 2);
        EXPECT_EQ(newStr[2], 1);

    }

    TEST(MonomialSubRule, MatchBBAtoNothing_ABBA) {
        std::vector<oper_name_t> sampleStr{1, 2, 2, 1};

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{2, 2, 1},
                                     std::vector<oper_name_t>{}};

        ASSERT_EQ(msr.Delta, -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin() + 1);

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 1);
    }

    TEST(MonomialSubRule, MatchBBAtoNothing_BBAB) {
        std::vector<oper_name_t> sampleStr{2, 2, 1, 2};

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{2, 2, 1},
                                     std::vector<oper_name_t>{}};

        ASSERT_EQ(msr.Delta, -3);

        auto match = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        ASSERT_EQ(match, sampleStr.cbegin());

        auto newStr = msr.apply_match_with_hint(sampleStr, match);
        ASSERT_EQ(newStr.size(), 1);
        EXPECT_EQ(newStr[0], 2);
    }

    TEST(MonomialSubRule, AllMatches_ABtoNull_in_ABAB) {
        AlgebraicContext context{3}; // 0, 1, 2
        RawSequenceBook rsb{context};
        rsb.generate(4);

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{1, 2},
                                     std::vector<oper_name_t>{}};

        std::vector<oper_name_t> sampleStr{1, 2, 1, 2};
        auto ssWhere = rsb.where(sampleStr);
        auto abWhere = rsb.where({1, 2});
        ASSERT_NE(ssWhere, nullptr);
        ASSERT_NE(abWhere, nullptr);
        ASSERT_LT(abWhere->raw_id, ssWhere->raw_id);

        std::vector<SymbolPair> output_ids;
        size_t count = msr.all_matches(output_ids, rsb, *ssWhere);
        EXPECT_EQ(count, 2);
        ASSERT_EQ(output_ids.size(), 2);

        ASSERT_EQ(output_ids.size(), 2);
        EXPECT_EQ(output_ids[0].left_id, abWhere->raw_id);
        EXPECT_EQ(output_ids[0].right_id, ssWhere->raw_id);
        EXPECT_EQ(output_ids[1].left_id, abWhere->raw_id);
        EXPECT_EQ(output_ids[1].right_id, ssWhere->raw_id);
    }

    TEST(MonomialSubRule, AllMatches_ABtoA_in_ABAB) {
        AlgebraicContext context{3}; // 0, 1, 2
        RawSequenceBook rsb{context};
        rsb.generate(4);

        MonomialSubstitutionRule msr{std::vector<oper_name_t>{1, 2},
                                     std::vector<oper_name_t>{1}};

        std::vector<oper_name_t> sampleStr{1, 2, 1, 2};
        auto ssWhere = rsb.where(sampleStr);
        auto aabWhere = rsb.where({1, 1, 2});
        auto abaWhere = rsb.where({1, 2, 1});
        ASSERT_NE(ssWhere, nullptr);
        ASSERT_NE(aabWhere, nullptr);
        ASSERT_NE(abaWhere, nullptr);
        ASSERT_NE(aabWhere->raw_id, abaWhere->raw_id);

        // Because symbol pair sorts...!
        ASSERT_LE(aabWhere->raw_id, ssWhere->raw_id);
        ASSERT_LE(abaWhere->raw_id, ssWhere->raw_id);


        std::vector<SymbolPair> output_ids;
        size_t count = msr.all_matches(output_ids, rsb, *ssWhere);
        EXPECT_EQ(count, 2);
        ASSERT_EQ(output_ids.size(), 2);
        EXPECT_EQ(output_ids[0].left_id, aabWhere->raw_id);
        EXPECT_EQ(output_ids[0].right_id, ssWhere->raw_id);
        EXPECT_EQ(output_ids[1].left_id, abaWhere->raw_id);
        EXPECT_EQ(output_ids[1].right_id, ssWhere->raw_id);
    }

}