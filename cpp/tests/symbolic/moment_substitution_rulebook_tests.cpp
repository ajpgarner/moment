/**
 * moment_substitution_rulebook_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "symbolic/moment_substitution_rule.h"
#include "symbolic/moment_substitution_rulebook.h"

namespace Moment::Tests {

    void assert_matching_rules(const MomentSubstitutionRulebook& book,
                               const std::vector<MomentSubstitutionRule>& expected) {
        ASSERT_EQ(book.size(), expected.size());
        ASSERT_EQ(book.empty(), expected.empty());

        size_t rule_number = 0;
        auto expected_iter = expected.begin();
        for (const auto& [id, rule] : book) {
            EXPECT_EQ(id, expected_iter->LHS()) << "Rule #" << rule_number;
            EXPECT_EQ(rule.LHS(), expected_iter->LHS()) << "Rule #" << rule_number;
            EXPECT_EQ(rule.RHS(), expected_iter->RHS()) << "Rule #" << rule_number;
            ++expected_iter;
            ++rule_number;
        }
    }


    TEST(Symbolic_MomentSubstitutionRulebook, Construct_Empty) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Inject) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare rulebook with single direct rule
        MomentSubstitutionRulebook book{table};
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{5, SymbolCombo::Zero()}));
        ASSERT_EQ(book.size(), 1);
        EXPECT_FALSE(book.empty());
        auto rule_iter = book.begin();
        ASSERT_NE(rule_iter, book.end());
        const auto& rule = *rule_iter;
        EXPECT_EQ(rule.first, 5);
        EXPECT_EQ(rule.second.LHS(), 5);
        EXPECT_EQ(rule.second.RHS(), SymbolCombo::Zero());
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Collides_Empty) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        MomentSubstitutionRule msr{5, SymbolCombo::Zero()};

        ASSERT_FALSE(book.collides(msr));
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Collides_InMIddle) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        book.inject(MomentSubstitutionRule{2, SymbolCombo::Zero()});
        book.inject(MomentSubstitutionRule{3, SymbolCombo::Zero()});
        book.inject(MomentSubstitutionRule{5, SymbolCombo::Zero()});

        EXPECT_FALSE(book.collides(MomentSubstitutionRule{4, SymbolCombo::Zero()}));
        EXPECT_TRUE(book.collides(MomentSubstitutionRule{3, SymbolCombo::Zero()}));
    }


    TEST(Symbolic_MomentSubstitutionRulebook, CollidesAtEnd_Empty) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        MomentSubstitutionRule msr{5, SymbolCombo::Zero()};

        ASSERT_FALSE(book.collides_at_end(msr));
    }

    TEST(Symbolic_MomentSubstitutionRulebook, CollidesAtEnd_OneRule) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{5, SymbolCombo::Zero()}));

        EXPECT_TRUE(book.collides_at_end(MomentSubstitutionRule{5, SymbolCombo::Scalar(1.0)}));
        EXPECT_FALSE(book.collides_at_end(MomentSubstitutionRule{6, SymbolCombo::Scalar(1.0)}));
    }


    TEST(Symbolic_MomentSubstitutionRulebook, Reduce_Empty) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();
        ASSERT_TRUE(book.empty());

        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}})),
                              factory({SymbolExpression{3, 1.0}})); // b -> b
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}, SymbolExpression{2, 0.5}})),
                              factory({SymbolExpression{3, 1.0}, SymbolExpression{2, 0.5}}));// b + 0.5a -> b + 0.5a

    }

    TEST(Symbolic_MomentSubstitutionRulebook, Reduce_OneRule) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{5, SymbolCombo::Zero()})); // ab -> 0 (inferred: ba -> 0)
        ASSERT_FALSE(book.empty());

        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0}})),
                              SymbolCombo::Zero()); // ab -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0, true}})),
                              SymbolCombo::Zero()); // ba -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0, true}, SymbolExpression{2, 1.0}})),
                              factory({SymbolExpression{2, 1.0}})); // ba + a -> a
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}})),
                              factory({SymbolExpression{3, 1.0}})); // b -> b
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRules) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{5, factory({SymbolExpression{3, 0.5}})})); // ab -> 0.5 b
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{2, SymbolCombo::Zero()})); // a -> 0
        ASSERT_EQ(book.size(), 2);


        // 0 -> 0
        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero());

        // ab -> 0.5 b
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 1.0}})),
                              factory({SymbolExpression{3, 0.5}}));

        // 2a -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{2, 2.0}})),
                              SymbolCombo::Zero());

        // 4ab + a + 5 -> 2b + 5
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 4.0}, SymbolExpression{2, 1.0}, SymbolExpression{1, 5.0}})),
                              factory({SymbolExpression{3, 2.0}, SymbolExpression{1, 5.0}}));
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRulesOverap) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{5, factory({SymbolExpression{3, 0.5}, SymbolExpression{1, 1.0}})})); // ab -> 0.5 b + 1
        ASSERT_TRUE(book.inject(MomentSubstitutionRule{2, SymbolCombo::Scalar(1.0)})); // a -> 1
        ASSERT_EQ(book.size(), 2);

        // 0 -> 0
        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero());

        // ab -> 0.5 b + 1
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 1.0}})),
                              factory({SymbolExpression{3, 0.5}, SymbolExpression{1, 1.0}}));

        // 2a -> 2
        EXPECT_EQ(book.reduce(factory({SymbolExpression{2, 2.0}})),
                              SymbolCombo::Scalar(2.0));

        // 4ab + a + 5 -> 2b + 10
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 4.0}, SymbolExpression{2, 1.0}, SymbolExpression{1, 5.0}})),
                              factory({SymbolExpression{3, 2.0}, SymbolExpression{1, 10.0}}));
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_Bto0) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(3, 1.0)})); // <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},
                                     MomentSubstitutionRule{3, SymbolCombo::Zero()}});

    }

    TEST(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_BtoA) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(3, 1.0), SymbolExpression(2, -1.0)})); // <b> - <a> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},
                                     MomentSubstitutionRule{3, SymbolCombo::Zero()}});

    }

    TEST(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAtoB) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(3, -1.0)})); // <aa> - <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{3, factory({SymbolExpression{2, 1.0}})},   // <b> -> <a>
                                     MomentSubstitutionRule{4, factory({SymbolExpression{2, 1.0}})}}); // <aa> -> <a>
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -2.0)})); // <aa> - 2<a> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},   // <a> -> 0
                                     MomentSubstitutionRule{4, SymbolCombo::Zero()}}); // <aa> -> 0
    }

    TEST(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A_AtoId) {

        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{table};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -2.0)})); // <aa> - 2<a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0), SymbolExpression(1, -1.0)})); // <a> - 1 = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &table);
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        EXPECT_THROW(book.complete(), errors::invalid_moment_rule);
    }
}