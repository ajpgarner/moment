/**
 * moment_substitution_rulebook_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "matrix/monomial_matrix.h"
#include "matrix/substituted_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "symbolic/moment_substitution_rule.h"
#include "symbolic/moment_substitution_rulebook.h"

#include "symbolic_matrix_helpers.h"

namespace Moment::Tests {
    namespace {
        void assert_matching_rules(const MomentSubstitutionRulebook &book,
                                   const std::vector<MomentSubstitutionRule> &expected) {
            ASSERT_EQ(book.size(), expected.size());
            ASSERT_EQ(book.empty(), expected.empty());

            size_t rule_number = 0;
            auto expected_iter = expected.begin();
            for (const auto &[id, rule]: book) {
                EXPECT_EQ(id, expected_iter->LHS()) << "Rule #" << rule_number;
                EXPECT_EQ(rule.LHS(), expected_iter->LHS()) << "Rule #" << rule_number;
                EXPECT_EQ(rule.RHS(), expected_iter->RHS()) << "Rule #" << rule_number;
                ++expected_iter;
                ++rule_number;
            }
        }
    }

    class Symbolic_MomentSubstitutionRulebook : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;
        std::unique_ptr<SymbolComboFactory> factory_ptr;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2)
            );
            ams_ptr->generate_dictionary(2); // e, a, b, aa, ab (ba), bb
            factory_ptr = std::make_unique<SymbolComboFactory>(ams_ptr->Symbols());
        }

        [[nodiscard]] const Algebraic::AlgebraicContext& get_context() const noexcept {
            return this->ams_ptr->AlgebraicContext();
        };

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ams_ptr->Symbols(); };

        [[nodiscard]] const SymbolComboFactory& get_factory() const noexcept { return *this->factory_ptr; };

    };


    TEST_F(Symbolic_MomentSubstitutionRulebook, Construct_Empty) {

        // Prepare trivial rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Inject) {

        // Prepare rulebook with single direct rule
        MomentSubstitutionRulebook book{this->get_symbols()};
        ASSERT_TRUE(book.inject(5, SymbolCombo::Zero()));
        ASSERT_EQ(book.size(), 1);
        EXPECT_FALSE(book.empty());
        auto rule_iter = book.begin();
        ASSERT_NE(rule_iter, book.end());
        const auto& rule = *rule_iter;
        EXPECT_EQ(rule.first, 5);
        EXPECT_EQ(rule.second.LHS(), 5);
        EXPECT_EQ(rule.second.RHS(), SymbolCombo::Zero());
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Collides_Empty) {

        MomentSubstitutionRulebook book{this->get_symbols()};
        MomentSubstitutionRule msr{5, SymbolCombo::Zero()};

        ASSERT_FALSE(book.collides(msr));
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Collides_InMiddle) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(2, SymbolCombo::Zero());
        book.inject(3, SymbolCombo::Zero());
        book.inject(5, SymbolCombo::Zero());

        EXPECT_FALSE(book.collides(MomentSubstitutionRule{4, SymbolCombo::Zero()}));
        EXPECT_TRUE(book.collides(MomentSubstitutionRule{3, SymbolCombo::Zero()}));
    }


    TEST_F(Symbolic_MomentSubstitutionRulebook, CollidesAtEnd_Empty) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        MomentSubstitutionRule msr{5, SymbolCombo::Zero()};

        ASSERT_FALSE(book.collides_at_end(msr));
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, CollidesAtEnd_OneRule) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        ASSERT_TRUE(book.inject(5, SymbolCombo::Zero()));

        EXPECT_TRUE(book.collides_at_end(MomentSubstitutionRule{5, SymbolCombo::Scalar(1.0)}));
        EXPECT_FALSE(book.collides_at_end(MomentSubstitutionRule{6, SymbolCombo::Scalar(1.0)}));
    }


    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_Empty) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.empty());

        EXPECT_EQ(book.reduce(SymbolExpression{3, 1.0}), factory({SymbolExpression{3, 1.0}}));
        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}})),
                              factory({SymbolExpression{3, 1.0}})); // b -> b
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}, SymbolExpression{2, 0.5}})),
                              factory({SymbolExpression{3, 1.0}, SymbolExpression{2, 0.5}}));// b + 0.5a -> b + 0.5a

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_OneRule) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, SymbolCombo::Zero())); // ab -> 0 (inferred: ba -> 0)
        ASSERT_FALSE(book.empty());

        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0}})),
                              SymbolCombo::Zero()); // ab -> 0
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0, true}})),
                              SymbolCombo::Zero()); // ba -> 0
        EXPECT_EQ(book.reduce(SymbolExpression{5, 1.0}), SymbolCombo::Zero());
        EXPECT_EQ(book.reduce(factory({SymbolExpression{5, 2.0, true}, SymbolExpression{2, 1.0}})),
                              factory({SymbolExpression{2, 1.0}})); // ba + a -> a
        EXPECT_EQ(book.reduce(factory({SymbolExpression{3, 1.0}})),
                              factory({SymbolExpression{3, 1.0}})); // b -> b
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, factory({SymbolExpression{3, 0.5}}))); // ab -> 0.5 b
        ASSERT_TRUE(book.inject(2, SymbolCombo::Zero())); // a -> 0
        ASSERT_EQ(book.size(), 2);

        // 0 -> 0
        EXPECT_EQ(book.reduce(SymbolCombo::Zero()), SymbolCombo::Zero());

        // ab -> 0.5 b
        EXPECT_EQ(book.reduce({SymbolExpression{5, 1.0}}),
                  factory({SymbolExpression{3, 0.5}}));

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

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRulesOverap) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, factory({SymbolExpression{3, 0.5}, SymbolExpression{1, 1.0}}))); // ab -> 0.5 b + 1
        ASSERT_TRUE(book.inject(2, SymbolCombo::Scalar(1.0))); // a -> 1
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

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_EmptyRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        ASSERT_TRUE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<SymbolExpression>::StorageType matrix_data =
                {SymbolExpression(1, 1.0), SymbolExpression{4, {2.0, 3.0}},
                 SymbolExpression{4, {2.0, -3.0}, true}, SymbolExpression{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        auto output = book.reduce(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, input_mm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_MonomialRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(2, SymbolCombo::Scalar(0.5));

        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<SymbolExpression>::StorageType matrix_data =
                {SymbolExpression(1, 1.0), SymbolExpression{4, {2.0, 3.0}},
                 SymbolExpression{4, {2.0, -3.0}, true}, SymbolExpression{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        SquareMatrix<SymbolExpression>::StorageType ref_matrix_data =
                {SymbolExpression(1, 1.0), SymbolExpression{4, {2.0, 3.0}},
                 SymbolExpression{4, {2.0, -3.0}, true}, SymbolExpression{1, 2.0}};

        MonomialMatrix ref_mm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(ref_matrix_data)), true};

        auto output = book.reduce(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, ref_mm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(3, factory({SymbolExpression{2,-1.0}, SymbolExpression{1,1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<SymbolExpression>::StorageType matrix_data =
                {SymbolExpression(1, 1.0), SymbolExpression{4, {2.0, 3.0}},
                 SymbolExpression{4, {2.0, -3.0}, true}, SymbolExpression{3, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        SquareMatrix<SymbolCombo>::StorageType ref_matrix_data =
                {SymbolCombo{SymbolExpression(1, 1.0)},
                 SymbolCombo{SymbolExpression{4, {2.0, 3.0}}},
                 SymbolCombo{SymbolExpression{4, {2.0, -3.0}, true}},
                 factory({{SymbolExpression{1, 4.0}, SymbolExpression{2, -4.0}}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(ref_matrix_data))};

        auto output = book.reduce(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_PolyMatrix_EmptyRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        ASSERT_TRUE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<SymbolCombo>::StorageType matrix_data =
                {SymbolCombo{SymbolExpression(1, 1.0)},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, 3.0}}}},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, -3.0}, true}}},
                 SymbolCombo{SymbolExpression{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(matrix_data))};

        auto output = book.reduce(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, input_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_PolyMatrix_MonomialRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(2, SymbolCombo::Scalar(2.0));
        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<SymbolCombo>::StorageType matrix_data =
                {SymbolCombo{SymbolExpression(1, 1.0)},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, 3.0}}}},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, -3.0}, true}}},
                 SymbolCombo{SymbolExpression{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(matrix_data))};

        SquareMatrix<SymbolCombo>::StorageType ref_matrix_data =
                {SymbolCombo{SymbolExpression(1, 1.0)},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, 3.0}}}},
                 SymbolCombo{{SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, -3.0}, true}}},
                 SymbolCombo{SymbolExpression{1, 8.0}}};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(ref_matrix_data))};


        auto output = book.reduce(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_PolyMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(3, factory({SymbolExpression{2,-1.0}, SymbolExpression{1,1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());


        SquareMatrix<SymbolCombo>::StorageType matrix_data =
                {factory({SymbolExpression(1, 1.0)}),
                 factory({SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, 3.0}}}),
                 factory({SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, -3.0}, true}}),
                 factory({SymbolExpression{3, 4.0}})};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(matrix_data))};

        SquareMatrix<SymbolCombo>::StorageType ref_matrix_data =
                {factory({SymbolExpression(1, 1.0)}),
                 factory({SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, 3.0}}}),
                 factory({SymbolExpression{1, 2.0}, SymbolExpression{4, {2.0, -3.0}, true}}),
                 factory({SymbolExpression{2, -4.0}, SymbolExpression{1, 4.0}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(ref_matrix_data))};

        auto output = book.reduce(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_Bto0) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(3, 1.0)})); // <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},
                                     MomentSubstitutionRule{3, SymbolCombo::Zero()}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_BtoA) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(3, 1.0), SymbolExpression(2, -1.0)})); // <b> - <a> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},
                                     MomentSubstitutionRule{3, SymbolCombo::Zero()}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAtoB) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(3, -1.0)})); // <aa> - <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{3, factory({SymbolExpression{2, 1.0}})},   // <b> -> <a>
                                     MomentSubstitutionRule{4, factory({SymbolExpression{2, 1.0}})}}); // <aa> -> <a>
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -2.0)})); // <aa> - 2<a> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},   // <a> -> 0
                                     MomentSubstitutionRule{4, SymbolCombo::Zero()}}); // <aa> -> 0
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A_AtoId) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<SymbolCombo> raw_combos;
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(4, 1.0), SymbolExpression(2, -2.0)})); // <aa> - 2<a> = 0
        raw_combos.emplace_back(factory({SymbolExpression(2, 1.0), SymbolExpression(1, -1.0)})); // <a> - 1 = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        EXPECT_THROW(book.complete(), errors::invalid_moment_rule);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_FromMap) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::map<symbol_name_t, double> raw_assignments;
        raw_assignments.insert(std::make_pair(2, 0.0)); // <a> = 0
        raw_assignments.insert(std::make_pair(3, 1.5)); // <b> = 1.5

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());

        book.add_raw_rules(raw_assignments);
        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, SymbolCombo::Zero()},
                                     MomentSubstitutionRule{3, SymbolCombo::Scalar(1.5)}});

    }

}