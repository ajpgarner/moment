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
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2)
            );
            ams_ptr->generate_dictionary(2); // e, a, b, aa, ab (ba), bb
            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ams_ptr->Symbols());
        }

        [[nodiscard]] Algebraic::AlgebraicMatrixSystem& get_system() const noexcept {
            return *this->ams_ptr;
        }

        [[nodiscard]] const Algebraic::AlgebraicContext& get_context() const noexcept {
            return this->ams_ptr->AlgebraicContext();
        };

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ams_ptr->Symbols(); };

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept { return *this->factory_ptr; };

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
        ASSERT_TRUE(book.inject(5, Polynomial()));
        ASSERT_EQ(book.size(), 1);
        EXPECT_FALSE(book.empty());
        auto rule_iter = book.begin();
        ASSERT_NE(rule_iter, book.end());
        const auto& rule = *rule_iter;
        EXPECT_EQ(rule.first, 5);
        EXPECT_EQ(rule.second.LHS(), 5);
        EXPECT_EQ(rule.second.RHS(), Polynomial());
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_Empty) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.empty());

        EXPECT_EQ(book.reduce(Monomial{3, 1.0}), factory({Monomial{3, 1.0}}));
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({Monomial{3, 1.0}})),
                              factory({Monomial{3, 1.0}})); // b -> b
        EXPECT_EQ(book.reduce(factory({Monomial{3, 1.0}, Monomial{2, 0.5}})),
                              factory({Monomial{3, 1.0}, Monomial{2, 0.5}}));// b + 0.5a -> b + 0.5a
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_OneRule) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, Polynomial())); // ab -> 0 (inferred: ba -> 0)
        ASSERT_FALSE(book.empty());

        EXPECT_EQ(book.reduce(Polynomial()), Polynomial()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({Monomial{5, 2.0}})),
                  Polynomial()); // ab -> 0
        EXPECT_EQ(book.reduce(factory({Monomial{5, 2.0, true}})),
                  Polynomial()); // ba -> 0
        EXPECT_EQ(book.reduce(Monomial{5, 1.0}), Polynomial());
        EXPECT_EQ(book.reduce(factory({Monomial{5, 2.0, true}, Monomial{2, 1.0}})),
                              factory({Monomial{2, 1.0}})); // ba + a -> a
        EXPECT_EQ(book.reduce(factory({Monomial{3, 1.0}})),
                              factory({Monomial{3, 1.0}})); // b -> b
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, factory({Monomial{3, 0.5}}))); // ab -> 0.5 b
        ASSERT_TRUE(book.inject(2, Polynomial())); // a -> 0
        ASSERT_EQ(book.size(), 2);

        // 0 -> 0
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial());

        // ab -> 0.5 b
        EXPECT_EQ(book.reduce({Monomial{5, 1.0}}),
                  factory({Monomial{3, 0.5}}));

        // ab -> 0.5 b
        EXPECT_EQ(book.reduce(factory({Monomial{5, 1.0}})),
                              factory({Monomial{3, 0.5}}));

        // 2a -> 0
        EXPECT_EQ(book.reduce(factory({Monomial{2, 2.0}})),
                  Polynomial());

        // 4ab + a + 5 -> 2b + 5
        EXPECT_EQ(book.reduce(factory({Monomial{5, 4.0}, Monomial{2, 1.0}, Monomial{1, 5.0}})),
                              factory({Monomial{3, 2.0}, Monomial{1, 5.0}}));
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_TwoRulesOverap) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};

        const auto& factory = book.Factory();
        ASSERT_TRUE(book.inject(5, factory({Monomial{3, 0.5}, Monomial{1, 1.0}}))); // ab -> 0.5 b + 1
        ASSERT_TRUE(book.inject(2, Polynomial::Scalar(1.0))); // a -> 1
        ASSERT_EQ(book.size(), 2);

        // 0 -> 0
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial());

        // ab -> 0.5 b + 1
        EXPECT_EQ(book.reduce(factory({Monomial{5, 1.0}})),
                              factory({Monomial{3, 0.5}, Monomial{1, 1.0}}));

        // 2a -> 2
        EXPECT_EQ(book.reduce(factory({Monomial{2, 2.0}})),
                  Polynomial::Scalar(2.0));

        // 4ab + a + 5 -> 2b + 10
        EXPECT_EQ(book.reduce(factory({Monomial{5, 4.0}, Monomial{2, 1.0}, Monomial{1, 5.0}})),
                              factory({Monomial{3, 2.0}, Monomial{1, 10.0}}));
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_EmptyRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        ASSERT_TRUE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, input_mm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_MonomialRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(2, Polynomial::Scalar(0.5));

        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        SquareMatrix<Monomial>::StorageType ref_matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{1, 2.0}};

        MonomialMatrix ref_mm{this->get_context(), this->get_symbols(),
                              std::make_unique<SquareMatrix<Monomial>>(2, std::move(ref_matrix_data)), true};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, ref_mm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_MonoMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(3, factory({Monomial{2, -1.0}, Monomial{1, 1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{3, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{Monomial{4, {2.0, 3.0}}},
                 Polynomial{Monomial{4, {2.0, -3.0}, true}},
                 factory({{Monomial{1, 4.0}, Monomial{2, -4.0}}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                          std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
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

        SquareMatrix<Polynomial>::StorageType matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, input_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_PolyMatrix_MonomialRules) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(2, Polynomial::Scalar(2.0));
        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Polynomial>::StorageType matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{1, 8.0}}};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};


        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Reduce_PolyMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentSubstitutionRulebook book{this->get_symbols()};
        book.inject(3, factory({Monomial{2, -1.0}, Monomial{1, 1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());


        SquareMatrix<Polynomial>::StorageType matrix_data =
                {factory({Monomial(1, 1.0)}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}),
                 factory({Monomial{3, 4.0}})};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(),
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {factory({Monomial(1, 1.0)}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}),
                 factory({Monomial{2, -4.0}, Monomial{1, 4.0}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(),
                                std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_Bto0) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({Monomial(3, 1.0)})); // <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, Polynomial()},
                                     MomentSubstitutionRule{3, Polynomial()}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_Ato0_BtoA) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({Monomial(3, 1.0), Monomial(2, -1.0)})); // <b> - <a> = 0

        ASSERT_EQ(raw_combos.back().last_id(), 3);
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentSubstitutionRule{2, Polynomial()},
                                     MomentSubstitutionRule{3, Polynomial()}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAtoB) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(3, -1.0)})); // <aa> - <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{3, factory({Monomial{2, 1.0}})},   // <b> -> <a>
                                     MomentSubstitutionRule{4, factory({Monomial{2, 1.0}})}}); // <aa> -> <a>
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -2.0)})); // <aa> - 2<a> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();
        ASSERT_FALSE(book.empty());
        ASSERT_EQ(book.size(), 2);

        assert_matching_rules(book, {MomentSubstitutionRule{2, Polynomial()},   // <a> -> 0
                                     MomentSubstitutionRule{4, Polynomial()}}); // <aa> -> 0
    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, Complete_AAtoA_AAto2A_AtoId) {
        // Prepare rulebook
        MomentSubstitutionRulebook book{this->get_symbols()};
        const auto& factory = book.Factory();

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -1.0)})); // <aa> - <a> = 0
        raw_combos.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -2.0)})); // <aa> - 2<a> = 0
        raw_combos.emplace_back(factory({Monomial(2, 1.0), Monomial(1, -1.0)})); // <a> - 1 = 0
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

        assert_matching_rules(book, {MomentSubstitutionRule{2, Polynomial()},
                                     MomentSubstitutionRule{3, Polynomial::Scalar(1.5)}});

    }



    TEST_F(Symbolic_MomentSubstitutionRulebook, CombineAndComplete_IntoEmpty) {
        // System
        MomentSubstitutionRulebook empty_book{this->get_symbols()};

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

        assert_matching_rules(book, {MomentSubstitutionRule{2, Polynomial()},
                                     MomentSubstitutionRule{3, Polynomial::Scalar(1.5)}});

        size_t new_rules = empty_book.combine_and_complete(std::move(book));
        EXPECT_EQ(new_rules, 2);
        assert_matching_rules(empty_book, {MomentSubstitutionRule{2, Polynomial()},
                                           MomentSubstitutionRule{3, Polynomial::Scalar(1.5)}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, CombineAndComplete_Trivial) {
        // System
        MomentSubstitutionRulebook book_one{this->get_symbols()};

        std::map<symbol_name_t, double> raw_assignments_one;
        raw_assignments_one.insert(std::make_pair(2, 0.0)); // <a> = 0


        book_one.add_raw_rules(raw_assignments_one);
        book_one.complete();

        assert_matching_rules(book_one, {MomentSubstitutionRule{2, Polynomial()}});

        // Prepare rulebook
        MomentSubstitutionRulebook book_two{this->get_symbols()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(3, 1.5)); // <b> = 1.5
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();

        assert_matching_rules(book_two, {MomentSubstitutionRule{3, Polynomial::Scalar(1.5)}});

        size_t new_rules = book_one.combine_and_complete(std::move(book_two));
        EXPECT_EQ(new_rules, 1);
        assert_matching_rules(book_one, {MomentSubstitutionRule{2, Polynomial()},
                                         MomentSubstitutionRule{3, Polynomial::Scalar(1.5)}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, CombineAndComplete_WithRewrite) {
        // Prepare first rulebook <AA> -> <A>
        MomentSubstitutionRulebook book_one{this->get_symbols()};
        auto& factory = this->get_factory();

        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -0.5)})); // <aa> - 0.5<a> = 0
        book_one.add_raw_rules(std::move(raw_combos_one));
        book_one.complete();

        assert_matching_rules(book_one, {MomentSubstitutionRule{4, factory({Monomial(2, 0.5)})}});

        // Prepare second rulebook <A> -> 0.5
        MomentSubstitutionRulebook book_two{this->get_symbols()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(2, 0.5)); // <a> = 0.5
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();
        assert_matching_rules(book_two, {MomentSubstitutionRule{2, Polynomial::Scalar(0.5)}});

        size_t new_rules = book_one.combine_and_complete(std::move(book_two));
        EXPECT_EQ(new_rules, 1);
        assert_matching_rules(book_one, {MomentSubstitutionRule{2, Polynomial::Scalar(0.5)},
                                         MomentSubstitutionRule{4, Polynomial::Scalar(0.25)}});

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, CombineAndComplete_FailBadRule) {
        // Prepare first rulebook <AA> -> <A>
        MomentSubstitutionRulebook book_one{this->get_symbols()};
        auto& factory = this->get_factory();

        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_one.add_raw_rules(std::move(raw_combos_one));
        book_one.complete();

        assert_matching_rules(book_one, {MomentSubstitutionRule{4, Polynomial::Scalar(0.5)}});

        // Prepare second rulebook <A> -> 0.5
        MomentSubstitutionRulebook book_two{this->get_symbols()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(4, 0.25)); // <aa> = 0.25
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();
        assert_matching_rules(book_two, {MomentSubstitutionRule{4, Polynomial::Scalar(0.25)}});

        EXPECT_THROW(book_one.combine_and_complete(std::move(book_two)), errors::invalid_moment_rule);

        assert_matching_rules(book_one, {MomentSubstitutionRule{4, Polynomial::Scalar(0.5)}});
        EXPECT_FALSE(book_one.pending_rules());

    }

    TEST_F(Symbolic_MomentSubstitutionRulebook, CloneMomentMatrix) {
        // Build unlinked pair (uninflated)
        auto& ams = this->get_system();
        const auto& context = this->get_context();
        auto& symbols = this->get_symbols();

        // Get operator names
        ASSERT_EQ(context.size(), 2);
        oper_name_t op_a = 0;
        oper_name_t op_b = 1;

        // Make moment matrix, then find symbols
        auto [mm_id, moment_matrix] = ams.create_moment_matrix(1);
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(context));
        auto id_a = find_or_fail(symbols, OperatorSequence{{op_a}, context});
        auto id_aa = find_or_fail(symbols, OperatorSequence{{op_a, op_a}, context});
        auto id_b = find_or_fail(symbols, OperatorSequence{{op_b}, context});
        auto id_bb = find_or_fail(symbols, OperatorSequence{{op_b, op_b}, context});
        auto id_ab = find_or_fail(symbols, OperatorSequence{{op_a, op_b}, context});

        std::set all_symbols{id_e, id_a, id_aa, id_b, id_bb, id_ab};
        ASSERT_EQ(all_symbols.size(), 6);

        std::vector<Monomial> ref_mm_data
                = {Monomial{1}, Monomial{id_a}, Monomial{id_b},
                   Monomial{id_a}, Monomial{id_aa}, Monomial{id_ab},
                   Monomial{id_b}, Monomial{id_ab, 1.0, true}, Monomial{id_bb}};
        MonomialMatrix ref_mm{context, symbols,
                              std::make_unique<SquareMatrix<Monomial>>(3, std::move(ref_mm_data)), true};

        compare_symbol_matrices(moment_matrix, ref_mm, "Moment matrix");

        // Build substitutions of just A
        auto [rb_id, book] = ams.add_rulebook(std::make_unique<MomentSubstitutionRulebook>(this->get_symbols()));
        book.inject(id_a, Polynomial::Scalar(2.0)); // A -> 2
        book.inject(id_b, Polynomial::Scalar(3.0)); // B -> 3
        book.infer_additional_rules_from_factors(ams);

        // Rewrite moment matrix with known values
        auto [sub_id, sub_matrix] = ams.create_substituted_matrix(mm_id, rb_id);

        // Test matrix object is unique
        ASSERT_NE(mm_id, sub_id);
        ASSERT_NE(&moment_matrix, &sub_matrix);
        ASSERT_TRUE(sub_matrix.is_monomial());

        // Symbol matrix should have with a replaced by 2.0
        ASSERT_EQ(sub_matrix.Dimension(), 3);
        const auto& sub_symbols = dynamic_cast<const MonomialMatrix&>(sub_matrix).SymbolMatrix;
        EXPECT_EQ(sub_symbols[0][0], Monomial(id_e));
        EXPECT_EQ(sub_symbols[0][1], Monomial(id_e, 2.0));
        EXPECT_EQ(sub_symbols[0][2], Monomial(id_e, 3.0));
        EXPECT_EQ(sub_symbols[1][0], Monomial(id_e, 2.0));
        EXPECT_EQ(sub_symbols[1][1], Monomial(id_aa, 1.0));
        EXPECT_EQ(sub_symbols[1][2], Monomial(id_ab, 1.0));
        EXPECT_EQ(sub_symbols[2][0], Monomial(id_e, 3.0));
        EXPECT_EQ(sub_symbols[2][1], Monomial(id_ab, 1.0, true));
        EXPECT_EQ(sub_symbols[2][2], Monomial(id_bb));

        // Check aliasing/caching
        const auto& sub_matrix_alias = ams.SubstitutedMatrix(mm_id, rb_id);
        EXPECT_EQ(&sub_matrix_alias.context, &context);
        ASSERT_EQ(&sub_matrix_alias, &sub_matrix);
    }
}