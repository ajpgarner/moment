/**
 * rulebook_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_precontext.h"
#include "scenarios/algebraic/operator_rulebook.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Scenarios_Algebraic_Rulebook, Empty) {
        AlgebraicPrecontext apc{1};
        OperatorRulebook rules{apc};
        EXPECT_EQ(rules.size(), 0);
        EXPECT_TRUE(rules.rules().empty());
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_ToEmpty) {
        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;
        OperatorRulebook rules{apc};
        EXPECT_EQ(rules.size(), 0);
        OperatorRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        ASSERT_EQ(rules.size(), 1);

        auto theRule = rules.rules().find(hasher({0, 1}));
        ASSERT_NE(theRule, rules.rules().cend());
        EXPECT_EQ(theRule->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRule->second.RHS(), HashedSequence({0}, hasher));

        EXPECT_EQ(theRule->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_ToNonEmpty) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;

        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 1);

        OperatorRule msr{HashedSequence{{0, 2}, hasher}, HashedSequence{{1}, hasher, SequenceSignType::Negative}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher({0, 1}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleA->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleB = rules.rules().find(hasher({0, 2}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({0, 2}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({1}, hasher, SequenceSignType::Negative));
        EXPECT_EQ(theRuleB->second.rule_sign(), SequenceSignType::Negative);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_Redundant) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        
        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 1);

        OperatorRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 0);
        EXPECT_EQ(rules.size(), 1);

        auto theRuleA = rules.rules().find(hasher({0, 1}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleA->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_ImpliesZero) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 1);

        OperatorRule msr{HashedSequence{{0, 1}, hasher}, HashedSequence{{0}, hasher, SequenceSignType::Negative}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        //EXPECT_TRUE(rules.complete(10));
        EXPECT_EQ(rules.size(), 2) << rules;

        auto theRuleA = rules.rules().find(hasher({0}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence(true));
        EXPECT_EQ(theRuleA->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleAB = rules.rules().find(hasher({0, 1}));
        ASSERT_NE(theRuleAB, rules.rules().cend()) << rules;
        EXPECT_EQ(theRuleAB->second.LHS(), HashedSequence({0, 1}, hasher));
        EXPECT_EQ(theRuleAB->second.RHS(), HashedSequence(true));
        EXPECT_EQ(theRuleAB->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_CtoB_CtoA) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 1);

        OperatorRule msr{HashedSequence{{2}, hasher}, HashedSequence{{0}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher({2}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleA->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleB = rules.rules().find(hasher({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleB->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_CtoA_CtoB) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        
        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 1);

        OperatorRule msr{HashedSequence{{2}, hasher}, HashedSequence{{1}, hasher}};
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 2);

        auto theRuleA = rules.rules().find(hasher({2}));
        ASSERT_NE(theRuleA, rules.rules().cend());
        EXPECT_EQ(theRuleA->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleA->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleA->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleB = rules.rules().find(hasher({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleB->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, AddRule_Cascade) {
        AlgebraicPrecontext apc{4, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr_list;
        msr_list.emplace_back(HashedSequence{{3}, hasher},
                         HashedSequence{{2}, hasher}); // D -> C
        msr_list.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{0}, hasher}); // C -> A
        OperatorRulebook rules{apc, msr_list};
        EXPECT_EQ(rules.size(), 2);

        OperatorRule msr{HashedSequence{{3}, hasher}, HashedSequence{{1}, hasher}}; // D -> B
        EXPECT_EQ(rules.add_rule(msr), 1);
        EXPECT_EQ(rules.size(), 3);

        auto theRuleD = rules.rules().find(hasher({3}));
        ASSERT_NE(theRuleD, rules.rules().cend());
        EXPECT_EQ(theRuleD->second.LHS(), HashedSequence({3}, hasher));
        EXPECT_EQ(theRuleD->second.RHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleD->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleC = rules.rules().find(hasher({2}));
        ASSERT_NE(theRuleC, rules.rules().cend());
        EXPECT_EQ(theRuleC->second.LHS(), HashedSequence({2}, hasher));
        EXPECT_EQ(theRuleC->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleC->second.rule_sign(), SequenceSignType::Positive);

        auto theRuleB = rules.rules().find(hasher({1}));
        ASSERT_NE(theRuleB, rules.rules().cend());
        EXPECT_EQ(theRuleB->second.LHS(), HashedSequence({1}, hasher));
        EXPECT_EQ(theRuleB->second.RHS(), HashedSequence({0}, hasher));
        EXPECT_EQ(theRuleB->second.rule_sign(), SequenceSignType::Positive);
    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_String) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr};

        auto simplified_string = rules.reduce(HashedSequence{{0, 1}, hasher});

        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_EQ(simplified_string[0], 0);
    }

     TEST(Scenarios_Algebraic_Rulebook, Reduce_StringRecursive) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr};

        auto simplified_string = rules.reduce(HashedSequence{{0, 1, 1, 1}, hasher});

        ASSERT_EQ(simplified_string.size(), 1); // 00
        EXPECT_FALSE(simplified_string.negated());
        EXPECT_EQ(simplified_string[0], 0);
    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_ABToZero_AB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        OperatorRulebook rules{apc, msr};

        auto simplified_string = rules.reduce(HashedSequence{{0, 1}, hasher});
        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_FALSE(simplified_string.negated());
        EXPECT_TRUE(simplified_string.zero());

        auto by_search_str = rules.reduce_via_search(HashedSequence{{0, 1}, hasher});
        EXPECT_EQ(by_search_str, simplified_string);
        EXPECT_FALSE(by_search_str.negated());
    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_ABToZero_ABBB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        OperatorRulebook rules{apc, msr};

        auto simplified_string = rules.reduce(HashedSequence{{0, 1, 1, 1}, hasher});

        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_TRUE(simplified_string.zero());
    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_ABToZero_BAB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        OperatorRulebook rules{apc, msr};

        auto simplified_string = rules.reduce(HashedSequence{{1, 0, 1}, hasher});

        ASSERT_EQ(simplified_string.size(), 0); // 0
        EXPECT_FALSE(simplified_string.negated());
        EXPECT_TRUE(simplified_string.zero());
    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_AntiCommutator) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        const auto& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{0, 1}, hasher, SequenceSignType::Negative}); // yx = -xy
        msr.emplace_back(HashedSequence{{2, 0}, hasher},
                         HashedSequence{{0, 2}, hasher, SequenceSignType::Negative}); // zx = -xz
        msr.emplace_back(HashedSequence{{2, 1}, hasher},
                         HashedSequence{{1, 2}, hasher, SequenceSignType::Negative}); // zy = -yz
        msr.emplace_back(HashedSequence{{0, 0}, hasher}, HashedSequence{false}); // xx = 1
        msr.emplace_back(HashedSequence{{1, 1}, hasher}, HashedSequence{false}); // yy = 1
        msr.emplace_back(HashedSequence{{2, 2}, hasher}, HashedSequence{false}); // zz = 1
        OperatorRulebook rules{apc, std::move(msr)};

        EXPECT_TRUE(rules.complete(0)) << rules;

        // X^2 -> 1
        auto simp_xxx = rules.reduce(HashedSequence{{0,0,0}, hasher});
        EXPECT_EQ(simp_xxx, HashedSequence({0}, hasher));
        EXPECT_NE(simp_xxx, HashedSequence({0}, hasher, SequenceSignType::Negative));

        // YX -> -XY
        auto simp_xy = rules.reduce(HashedSequence{{1, 0}, hasher});
        EXPECT_EQ(simp_xy, HashedSequence({0, 1}, hasher, SequenceSignType::Negative));

        // YXX -> -XYX -> XXY - > Y
        auto simp_yxx = rules.reduce(HashedSequence{{1, 0, 0}, hasher});
        EXPECT_EQ(simp_yxx, HashedSequence({1}, hasher));

        // ZYX -> -YZX -> YXZ -> -XYZ
        auto simp_zyx = rules.reduce(HashedSequence{{2, 1, 0}, hasher});
        EXPECT_EQ(simp_zyx, HashedSequence({0, 1, 2}, hasher, SequenceSignType::Negative));
    }

    TEST(Scenarios_Algebraic_Rulebook, ReduceInPlace_String) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;

        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr};

        HashedSequence string{{0, 1}, hasher};

        auto result = rules.reduce_in_place(string);

        EXPECT_EQ(result, OperatorRulebook::RawReductionResult::Match);
        ASSERT_EQ(string.size(), 1);
        EXPECT_EQ(string[0], 0);
        EXPECT_EQ(string.hash(), apc.hasher({0}));
    }

    TEST(Scenarios_Algebraic_Rulebook, ReduceInPlace_StringRecursive) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr};

        HashedSequence string{{0, 1, 1, 1}, hasher};

        auto result = rules.reduce_in_place(string);

        EXPECT_EQ(result, OperatorRulebook::RawReductionResult::Match);
        ASSERT_EQ(string.size(), 1);
        EXPECT_EQ(string[0], 0);
        EXPECT_EQ(string.hash(), apc.hasher({0}));
    }

    TEST(Scenarios_Algebraic_Rulebook, ReduceInPlace_ABToZero_ABBB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        OperatorRulebook rules{apc, msr};


        HashedSequence abbb{{0, 1, 1, 1}, hasher};
        auto result = rules.reduce_in_place(abbb);
        EXPECT_EQ(result, OperatorRulebook::RawReductionResult::SetToZero);
        EXPECT_EQ(abbb.size(), 0);
        EXPECT_TRUE(abbb.zero());
    }

    TEST(Scenarios_Algebraic_Rulebook, ReduceInPlace_ABToZero_BAB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher}, // AB = 0
                         HashedSequence{true});
        OperatorRulebook rules{apc, msr};

        HashedSequence bab{{1, 0, 1}, hasher};

        auto result = rules.reduce_in_place(bab);
        EXPECT_EQ(result, OperatorRulebook::RawReductionResult::SetToZero);
        EXPECT_EQ(bab.size(), 0);
        EXPECT_TRUE(bab.zero());
    }


    TEST(Scenarios_Algebraic_Rulebook, ReduceInPlace_PauliSet) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        const auto& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{0, 1}, hasher, SequenceSignType::Negative}); // yx = -xy
        msr.emplace_back(HashedSequence{{2, 0}, hasher},
                         HashedSequence{{0, 2}, hasher, SequenceSignType::Negative}); // zx = -xz
        msr.emplace_back(HashedSequence{{2, 1}, hasher},
                         HashedSequence{{1, 2}, hasher, SequenceSignType::Negative}); // zy = -yz
        msr.emplace_back(HashedSequence{{0, 0}, hasher}, HashedSequence{false}); // yx = -xy
        msr.emplace_back(HashedSequence{{1, 1}, hasher}, HashedSequence{false}); // zx = -xz
        msr.emplace_back(HashedSequence{{2, 2}, hasher}, HashedSequence{false}); // zy = -yz
        OperatorRulebook rules{apc, std::move(msr)};

        EXPECT_TRUE(rules.complete(0)) << rules;

        // X^2 -> 1
        HashedSequence simp_xxx{{0, 0, 0}, hasher};
        auto result_xxx = rules.reduce_in_place(simp_xxx);
        EXPECT_EQ(result_xxx, OperatorRulebook::RawReductionResult::Match);
        EXPECT_EQ(simp_xxx, HashedSequence({0}, hasher));

        // YX -> -XY
        HashedSequence simp_yx{{1, 0}, hasher};
        auto result_yz = rules.reduce_in_place(simp_yx);
        EXPECT_EQ(result_yz, OperatorRulebook::RawReductionResult::Match);
        EXPECT_EQ(simp_yx, HashedSequence({0, 1}, hasher, SequenceSignType::Negative));

        // YXX -> -XYX -> XXY - > Y
        HashedSequence simp_yxx{{1, 0, 0}, hasher};
        auto result_yxx = rules.reduce_in_place(simp_yxx);
        EXPECT_EQ(result_yxx, OperatorRulebook::RawReductionResult::Match);
        EXPECT_EQ(simp_yxx, HashedSequence({1}, hasher));

        // ZYX -> -YZX -> YXZ -> -XYZ
        HashedSequence simp_zyx{{2, 1, 0}, hasher};
        auto result_zyx = rules.reduce_in_place(simp_zyx);
        EXPECT_EQ(result_zyx, OperatorRulebook::RawReductionResult::Match);
        EXPECT_EQ(simp_zyx, HashedSequence({0, 1, 2}, hasher, SequenceSignType::Negative));
    }


    TEST(Scenarios_Algebraic_Rulebook, Reduce_Rule) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        OperatorRulebook rules{apc, msr};

        auto simplified_rule = rules.reduce(
                OperatorRule{HashedSequence{{0, 1}, hasher}, HashedSequence{{0, 0}, hasher}}
                );

        ASSERT_EQ(simplified_rule.LHS().size(), 2); // 00
        EXPECT_EQ(simplified_rule.LHS()[0], 0);
        EXPECT_EQ(simplified_rule.LHS()[1], 0);

        ASSERT_EQ(simplified_rule.RHS().size(), 1); // 01 -> 0
        EXPECT_EQ(simplified_rule.RHS()[0], 0);

    }

    TEST(Scenarios_Algebraic_Rulebook, Reduce_RuleToZero) {
        AlgebraicPrecontext apc{4, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher}); // 2 = 1
        msr.emplace_back(HashedSequence{{3}, hasher},
                         HashedSequence{{1}, hasher, SequenceSignType::Negative}); // 3 = -1
        OperatorRulebook rules{apc, msr};

        auto simplified_rule = rules.reduce(
                OperatorRule{HashedSequence{{3}, hasher}, HashedSequence{{2}, hasher}}
                );

        // Rule reduces to 1 = -1 => 1 = [null]
        ASSERT_EQ(simplified_rule.LHS().size(), 1);
        EXPECT_EQ(simplified_rule.LHS()[0], 1);

        ASSERT_EQ(simplified_rule.RHS().size(), 0);
        EXPECT_TRUE(simplified_rule.RHS().zero());
    }

    TEST(Scenarios_Algebraic_Rulebook, ReduceRuleset_AACtoAAB_CtoB) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 2}, hasher},
                         HashedSequence{{0, 0, 1}, hasher});
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        OperatorRulebook rules{apc, msr};

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

    TEST(Scenarios_Algebraic_Rulebook, ReduceRuleset_CtoB_BtoA) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{2}, hasher},
                         HashedSequence{{1}, hasher});
        msr.emplace_back(HashedSequence{{1}, hasher},
                         HashedSequence{{0}, hasher});
        OperatorRulebook rules{apc, msr};

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

    TEST(Scenarios_Algebraic_Rulebook, AddConjugateRule) {
        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        OperatorRulebook rules{apc, msr};

        ASSERT_EQ(rules.rules().size(), 1);
        EXPECT_TRUE(rules.try_conjugation(rules.rules().begin()->second));
        EXPECT_EQ(rules.rules().size(), 2);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 1}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 0}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
    }

    TEST(Scenarios_Algebraic_Rulebook, ConjugateRuleset) {
        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        OperatorRulebook rules{apc, msr};

        ASSERT_EQ(rules.rules().size(), 1);

        EXPECT_EQ(rules.conjugate_ruleset(), 1);
        EXPECT_EQ(rules.rules().size(), 2);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 1}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 0}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
    }

    TEST(Scenarios_Algebraic_Rulebook, Complete_ABtoA_BAtoB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        OperatorRulebook rules{apc, msr};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.try_new_combination());
        ASSERT_EQ(rules.rules().size(), 3); // Should add 00 -> 0

        ASSERT_TRUE(rules.try_new_combination());
        ASSERT_EQ(rules.rules().size(), 4); // Should add 11 -> 1

        ASSERT_FALSE(rules.try_new_combination()); // No further confluences

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 1}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0}, hasher}),
                  HashedSequence({1}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1}, hasher}),
                  HashedSequence({1}, hasher, SequenceSignType::Positive));

        EXPECT_TRUE(rules.is_complete(false));
        EXPECT_FALSE(rules.is_complete(true));
    }

    TEST(Scenarios_Algebraic_Rulebook, Complete_AAAtoI_BBBtoI_ABABABtoI) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 0, 0}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{1, 1, 1}, hasher},
                         HashedSequence{{}, hasher});
        msr.emplace_back(HashedSequence{{0, 1, 0, 1, 0, 1}, hasher},
                         HashedSequence{{}, hasher});
        OperatorRulebook rules{apc, msr};

        EXPECT_FALSE(rules.is_complete());

        ASSERT_TRUE(rules.complete(20));
        EXPECT_EQ(rules.rules().size(), 8);

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0, 0}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 1}, hasher}),
                  HashedSequence({}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0, 1, 0}, hasher}),
                  HashedSequence({0, 0, 1, 1}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1, 0, 0}, hasher}),
                  HashedSequence({0, 1, 0, 1}, hasher, SequenceSignType::Positive));

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(Scenarios_Algebraic_Rulebook, Complete_ABtoA_BAtoMinusB) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher, SequenceSignType::Negative});
        OperatorRulebook rules{apc, msr};

        EXPECT_FALSE(rules.is_complete());
        ASSERT_TRUE(rules.complete(10));

        // aa = -a; ab = a; ba = -b; bb = b
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Negative)) << rules;
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 1}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Positive)) << rules;
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 0}, hasher}),
                  HashedSequence({1}, hasher, SequenceSignType::Negative)) << rules;
        EXPECT_EQ(rules.reduce(HashedSequence{{1, 1}, hasher}),
                  HashedSequence({1}, hasher, SequenceSignType::Positive)) << rules;

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(Scenarios_Algebraic_Rulebook, HermitianComplete_ABtoA_BAtoB_Hermitian) {
        AlgebraicPrecontext apc{2};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{1}, hasher});
        OperatorRulebook rules{apc, msr};

        EXPECT_FALSE(rules.is_complete());

        rules.complete(10);
        ASSERT_EQ(rules.rules().size(), 2); // Should end up with 1 -> 0 and 00 -> 0.
        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1}, hasher}),
                  HashedSequence({0}, hasher, SequenceSignType::Positive));

        EXPECT_TRUE(rules.is_complete());
    }

    TEST(Scenarios_Algebraic_Rulebook, HermitianComplete_ABtoA_BCtoB_CAtoC) {
        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher});
        msr.emplace_back(HashedSequence{{1, 2}, hasher},
                         HashedSequence{{1}, hasher});
        msr.emplace_back(HashedSequence{{2, 0}, hasher},
                         HashedSequence{{2}, hasher});
        OperatorRulebook rules{apc, msr};

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

        EXPECT_EQ(rules.reduce(HashedSequence{{0, 0}, hasher}),HashedSequence({0}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{1}, hasher}), HashedSequence({0}, hasher, SequenceSignType::Positive));
        EXPECT_EQ(rules.reduce(HashedSequence{{2}, hasher}), HashedSequence({0}, hasher, SequenceSignType::Positive));

        EXPECT_TRUE(rules.is_complete());
    }


    TEST(Scenarios_Algebraic_Rulebook, GenerateCommutators) {
        AlgebraicPrecontext apc{3};
        const ShortlexHasher& hasher = apc.hasher;
        auto comVec = OperatorRulebook::commutator_rules(apc);
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

    TEST(Scenarios_Algebraic_Rulebook, GenerateNormalRules_Bunched) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Bunched};
        const ShortlexHasher& hasher = apc.hasher;
        auto normVec = OperatorRulebook::normal_rules(apc);
        ASSERT_EQ(normVec.size(), 3);

        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(normVec[i].LHS().size(), 2) << i;
            ASSERT_EQ(normVec[i].RHS().size(), 2) << i;
        }

        EXPECT_EQ(normVec[0].LHS(),HashedSequence({3, 0}, hasher));
        EXPECT_EQ(normVec[0].RHS(),HashedSequence({0, 3}, hasher));

        EXPECT_EQ(normVec[1].LHS(),HashedSequence({4, 1}, hasher));
        EXPECT_EQ(normVec[1].RHS(),HashedSequence({1, 4}, hasher));

        EXPECT_EQ(normVec[2].LHS(),HashedSequence({5, 2}, hasher));
        EXPECT_EQ(normVec[2].RHS(),HashedSequence({2, 5}, hasher));

    }

    TEST(Scenarios_Algebraic_Rulebook, GenerateNormalRules_Interleaved) {
        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::Interleaved};
        const ShortlexHasher& hasher = apc.hasher;
        auto normVec = OperatorRulebook::normal_rules(apc);
        ASSERT_EQ(normVec.size(), 3);

        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(normVec[i].LHS().size(), 2) << i;
            ASSERT_EQ(normVec[i].RHS().size(), 2) << i;
        }

        EXPECT_EQ(normVec[0].LHS(),HashedSequence({1, 0}, hasher));
        EXPECT_EQ(normVec[0].RHS(),HashedSequence({0, 1}, hasher));

        EXPECT_EQ(normVec[1].LHS(),HashedSequence({3, 2}, hasher));
        EXPECT_EQ(normVec[1].RHS(),HashedSequence({2, 3}, hasher));

        EXPECT_EQ(normVec[2].LHS(),HashedSequence({5, 4}, hasher));
        EXPECT_EQ(normVec[2].RHS(),HashedSequence({4, 5}, hasher));

    }


    TEST(Scenarios_Algebraic_Rulebook, ImplyZero) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        const ShortlexHasher& hasher = apc.hasher;
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{1, 0}, hasher},
                         HashedSequence{{0, 1}, hasher, SequenceSignType::Negative}); // yx = -xy
        msr.emplace_back(HashedSequence{{0, 0}, hasher},
                         HashedSequence{{0}, hasher});          // xx = x
        OperatorRulebook rules{apc, std::move(msr)};
        EXPECT_TRUE(rules.complete(10));
        ASSERT_EQ(rules.size(), 3) << rules;
        auto ruleIter = rules.rules().begin();

        ASSERT_NE(ruleIter, rules.rules().end());
        const auto& ruleA = ruleIter->second;
        EXPECT_EQ(ruleA.LHS(), HashedSequence({0, 0}, hasher)) << ruleA;
        EXPECT_EQ(ruleA.RHS(), HashedSequence({0}, hasher)) << ruleA;
        EXPECT_FALSE(ruleA.implies_zero());

        ++ruleIter;
        ASSERT_NE(ruleIter, rules.rules().end());
        const auto& ruleB = ruleIter->second;
        EXPECT_EQ(ruleB.LHS(), HashedSequence({0, 1}, hasher)) << ruleB;
        EXPECT_EQ(ruleB.RHS(), HashedSequence(true)) << ruleB;
        EXPECT_TRUE(ruleB.implies_zero());

        ++ruleIter;
        ASSERT_NE(ruleIter, rules.rules().end());
        const auto& ruleC = ruleIter->second;
        EXPECT_EQ(ruleC.LHS(), HashedSequence({1, 0}, hasher)) << ruleB;
        EXPECT_EQ(ruleC.RHS(), HashedSequence(true)) << ruleB;
        EXPECT_TRUE(ruleC.implies_zero());

        ++ruleIter;
        EXPECT_EQ(ruleIter, rules.rules().end());
    }

}