/**
 * moment_rulebook_tests.cpp
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

#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "symbolic/rules/moment_rule.h"
#include "symbolic/rules/moment_rulebook.h"

#include "../symbolic_matrix_helpers.h"

#include "moment_rule_helpers.h"

#include <numbers>

namespace Moment::Tests {
    class Symbolic_MomentRulebook : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2), 10
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

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept {
            return this->ams_ptr->polynomial_factory();
        };

        void expect_approximately_equal(const Polynomial& LHS, const Polynomial& RHS) {
            expect_matching_polynomials("Polynomial", LHS, RHS, this->factory_ptr->zero_tolerance);
        }
    };


    TEST_F(Symbolic_MomentRulebook, Construct_Empty) {

        // Prepare trivial rulebook
        MomentRulebook book{this->get_system()};
        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());
    }

    TEST_F(Symbolic_MomentRulebook, Inject) {

        // Prepare rulebook with single direct rule
        MomentRulebook book{this->get_system()};
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

    TEST_F(Symbolic_MomentRulebook, Match_Empty) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
        ASSERT_TRUE(book.empty());

        Polynomial zero = Polynomial::Zero();
        auto [zero_rule, zero_match] = book.match(zero);
        EXPECT_EQ(zero_rule, book.end());
        EXPECT_EQ(zero_match, zero.end());

        Polynomial ab = factory({Monomial{5, 1.0}});
        auto [ab_rule, ab_match] = book.match(ab);
        EXPECT_EQ(ab_rule, book.end());
        EXPECT_EQ(ab_match, ab.end());

        Polynomial a_plus_ab = factory({Monomial{2, 1.0}, Monomial{5, 1.0}});
        auto [a_plus_ab_rule, a_plus_ab_match] = book.match(a_plus_ab);
        EXPECT_EQ(a_plus_ab_rule, book.end());
        EXPECT_EQ(a_plus_ab_match, a_plus_ab.end());
    }

    TEST_F(Symbolic_MomentRulebook, Match_OneRule) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
        ASSERT_TRUE(book.inject(5, Polynomial())); // ab -> 0 (inferred: ba -> 0)
        ASSERT_FALSE(book.empty());

        Polynomial zero = Polynomial::Zero();
        auto [zero_rule, zero_match] = book.match(zero);
        EXPECT_EQ(zero_rule, book.end());
        EXPECT_EQ(zero_match, zero.end());

        Polynomial ab = factory({Monomial{5, 1.0}});
        auto [ab_rule, ab_match] = book.match(ab);
        EXPECT_EQ(ab_rule, book.begin());
        EXPECT_EQ(ab_match, ab.begin());

        Polynomial a_plus_ab = factory({Monomial{2, 1.0}, Monomial{5, 1.0}});
        auto [a_plus_ab_rule, a_plus_ab_match] = book.match(a_plus_ab);
        EXPECT_EQ(a_plus_ab_rule, book.begin());
        EXPECT_EQ(a_plus_ab_match, a_plus_ab.begin()+1);
    }


    TEST_F(Symbolic_MomentRulebook, Reduce_Empty) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
        ASSERT_TRUE(book.empty());

        EXPECT_EQ(book.reduce(Monomial{3, 1.0}), factory({Monomial{3, 1.0}}));
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial()); // 0 -> 0
        EXPECT_EQ(book.reduce(factory({Monomial{3, 1.0}})),
                              factory({Monomial{3, 1.0}})); // b -> b
        EXPECT_EQ(book.reduce(factory({Monomial{3, 1.0}, Monomial{2, 0.5}})),
                              factory({Monomial{3, 1.0}, Monomial{2, 0.5}}));// b + 0.5a -> b + 0.5a
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_OneRule) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
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

    TEST_F(Symbolic_MomentRulebook, Reduce_TwoRules) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
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

    TEST_F(Symbolic_MomentRulebook, Reduce_TwoRulesOverap) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
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


    TEST_F(Symbolic_MomentRulebook, Reduce_NonorientableRule_Real) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory; // // e, a, b, aa, ab (ba), bb

        // Re(ab) - a = 0
        book.add_raw_rule(factory({Monomial{5, 0.5}, Monomial{5, 0.5, true}, Monomial{2, -1.0}}));
        book.complete();
        ASSERT_EQ(book.size(), 1);

        // 0 -> 0
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial());

        // ab -> iIm(ab) + a
        auto reduced_ab = book.reduce(factory({Monomial{5, 1.0}}));
        EXPECT_EQ(reduced_ab, factory({Monomial{5, std::complex{0.5, 0.0}},
                                      Monomial{5, std::complex{-0.5, 0.0}, true},
                                      Monomial{2, 1.0}}));

        // Check idempotence
        EXPECT_EQ(book.reduce(reduced_ab), reduced_ab);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_NonorientableRule_Imaginary) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
        // Im(ab) - a + b = 0
        book.add_raw_rule(factory({Monomial{5, std::complex{0.0, -0.5}},
                                   Monomial{5, std::complex{0.0, 0.5}, true}, Monomial{2, -1.0}, Monomial{3, 1.0}}));
        book.complete();
        ASSERT_EQ(book.size(), 1);

        // 0 -> 0
        EXPECT_EQ(book.reduce(Polynomial()), Polynomial());

        // ab -> Re(ab) + i(a - b)
        auto reduced_ab = book.reduce(factory({Monomial{5, 1.0}}));
        EXPECT_EQ(reduced_ab, factory({Monomial{5, std::complex{0.5, 0.0}},
                                      Monomial{5, std::complex{0.5, 0.0}, true},
                                      Monomial{2, std::complex{0.0, 1.0}},
                                      Monomial{3, std::complex{0.0, -1.0}}}));

        // Check idempotence
        EXPECT_EQ(book.reduce(reduced_ab), reduced_ab);
    }



    TEST_F(Symbolic_MomentRulebook, Reduce_MonoMatrix_EmptyRules) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;
        ASSERT_TRUE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, input_mm);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_MonoMatrix_MonomialRules) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;
        book.inject(2, Polynomial::Scalar(0.5));

        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{2, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        SquareMatrix<Monomial>::StorageType ref_matrix_data =
                {Monomial(1, 1.0), Monomial{4, {2.0, 3.0}},
                 Monomial{4, {2.0, -3.0}, true}, Monomial{1, 2.0}};

        MonomialMatrix ref_mm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                              std::make_unique<SquareMatrix<Monomial>>(2, std::move(ref_matrix_data)), true};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_TRUE(output->is_monomial());
        const auto& output_as_mm = dynamic_cast<const MonomialMatrix&>(*output);

        compare_symbol_matrices(output_as_mm, ref_mm);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_MonoMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentRulebook book{this->get_system()};
        book.inject(3, factory({Monomial{2, -1.0}, Monomial{1, 1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Monomial>::StorageType matrix_data =
                {Monomial(1, 1.0), Monomial{5, {2.0, 3.0}},
                 Monomial{5, {2.0, -3.0}, true}, Monomial{3, 4.0}};

        MonomialMatrix input_mm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                std::make_unique<SquareMatrix<Monomial>>(2, std::move(matrix_data)), true};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{Monomial{5, {2.0, 3.0}}},
                 Polynomial{Monomial{5, {2.0, -3.0}, true}},
                 factory({{Monomial{1, 4.0}, Monomial{2, -4.0}}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                          std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_mm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_PolyMatrix_EmptyRules) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;
        ASSERT_TRUE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Polynomial>::StorageType matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, input_pm);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_PolyMatrix_MonomialRules) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;
        book.inject(2, Polynomial::Scalar(2.0));
        ASSERT_FALSE(book.empty());
        ASSERT_TRUE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());

        SquareMatrix<Polynomial>::StorageType matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{2, 4.0}}};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {Polynomial{Monomial(1, 1.0)},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}},
                 Polynomial{{Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}},
                 Polynomial{Monomial{1, 8.0}}};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};


        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentRulebook, Reduce_PolyMatrix_PolynomialRules) {
        // Prepare rulebook
        const auto& factory = this->get_factory();
        MomentRulebook book{this->get_system()};
        book.inject(3, factory({Monomial{2, -1.0}, Monomial{1, 1.0}}));

        ASSERT_FALSE(book.empty());
        ASSERT_FALSE(book.is_monomial());
        ASSERT_TRUE(book.is_hermitian());


        SquareMatrix<Polynomial>::StorageType matrix_data =
                {factory({Monomial(1, 1.0)}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}),
                 factory({Monomial{3, 4.0}})};

        PolynomialMatrix input_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                  std::make_unique<SquareMatrix<Polynomial>>(2, std::move(matrix_data))};

        SquareMatrix<Polynomial>::StorageType ref_matrix_data =
                {factory({Monomial(1, 1.0)}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, 3.0}}}),
                 factory({Monomial{1, 2.0}, Monomial{4, {2.0, -3.0}, true}}),
                 factory({Monomial{2, -4.0}, Monomial{1, 4.0}})};

        PolynomialMatrix ref_pm{this->get_context(), this->get_symbols(), factory.zero_tolerance,
                                std::make_unique<SquareMatrix<Polynomial>>(2, std::move(ref_matrix_data))};

        auto output = book.create_substituted_matrix(this->get_symbols(), input_pm);
        ASSERT_TRUE(output);
        ASSERT_FALSE(output->is_monomial());
        const auto& output_as_pm = dynamic_cast<const PolynomialMatrix&>(*output);

        compare_symbol_matrices(output_as_pm, ref_pm);
    }

    TEST_F(Symbolic_MomentRulebook, Complete_Ato0_Bto0) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        std::vector<Polynomial> raw_combos;
        raw_combos.emplace_back(factory({Monomial(2, 1.0)})); // <a> = 0
        raw_combos.emplace_back(factory({Monomial(3, 1.0)})); // <b> = 0
        book.add_raw_rules(std::move(raw_combos));

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());
        EXPECT_EQ(book.size(), 0);
        EXPECT_EQ(book.begin(), book.end());

        book.complete();

        assert_matching_rules(book, {MomentRule{2, Polynomial()},
                                     MomentRule{3, Polynomial()}});

    }

    TEST_F(Symbolic_MomentRulebook, Complete_Ato0_BtoA) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

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

        assert_matching_rules(book, {MomentRule{2, Polynomial()},
                                     MomentRule{3, Polynomial()}});

    }

    TEST_F(Symbolic_MomentRulebook, Complete_AAtoA_AAtoB) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

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

        assert_matching_rules(book, {MomentRule{3, factory({Monomial{2, 1.0}})},   // <b> -> <a>
                                     MomentRule{4, factory({Monomial{2, 1.0}})}}); // <aa> -> <a>
    }

    TEST_F(Symbolic_MomentRulebook, Complete_AAtoA_AAto2A) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

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

        assert_matching_rules(book, {MomentRule{2, Polynomial()},   // <a> -> 0
                                     MomentRule{4, Polynomial()}}); // <aa> -> 0
    }

    TEST_F(Symbolic_MomentRulebook, Complete_AAtoA_AAto2A_AtoId) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

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

    TEST_F(Symbolic_MomentRulebook, Complete_RealAndImParts) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        std::vector<Polynomial> raw_combos; // Re(<ab>) = <a>; Im(<ab>) = <b>
        raw_combos.emplace_back(factory({Monomial{5, 0.5}, Monomial{5, 0.5, true}, Monomial{2, -1.0}}));
        raw_combos.emplace_back(factory({Monomial{5, std::complex{0.0, -0.5}},
                                         Monomial{5, std::complex{0.0, 0.5}, true}, Monomial{3, -1.0}}));
        book.add_raw_rules(std::move(raw_combos));
        book.complete();
        EXPECT_EQ(book.size(), 1);

        auto reduced_ab = book.reduce(Polynomial{Monomial{5, 1.0}});
        EXPECT_EQ(reduced_ab, factory({Monomial{2, 1.0}, Monomial{3, std::complex{0.0, 1.0}}}));
    }

    TEST_F(Symbolic_MomentRulebook, Complete_FullThenReal) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        // Force full rule into book
        book.inject(5, Polynomial::Scalar(std::complex{1.0, 1.0})); // <ab> = 1 + i

        std::vector<Polynomial> raw_combos; // Re(<ab>) = <a>;
        raw_combos.emplace_back(factory({Monomial{5, 0.5}, Monomial{5, 0.5, true}, Monomial{2, -1.0}}));
        book.add_raw_rules(std::move(raw_combos));
        book.complete();
        ASSERT_EQ(book.size(), 2);

        // Should map AB -> 1 + i
        auto reduced_ab = book.reduce(Polynomial{Monomial{5, 1.0}});
        EXPECT_EQ(reduced_ab, Polynomial::Scalar(std::complex{1.0, 1.0}));

        // Should map A -> 1
        EXPECT_EQ(book.reduce(Polynomial{Monomial{2}}), Polynomial::Scalar(std::complex{1.0, 0.0}));
    }


    TEST_F(Symbolic_MomentRulebook, Complete_RealThenFull) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        // Add rule to book.
        book.add_raw_rule(factory({Monomial{5, 0.5}, Monomial{5, 0.5, true}, Monomial{2, -1.0}}));
        book.complete();
        ASSERT_EQ(book.size(), 1);

        // Add another rule afterwards
        book.add_raw_rule(factory({Monomial{5, 1.0}, Monomial{1, std::complex{-1.0, -1.0}}}));
        book.complete();

        ASSERT_EQ(book.size(), 2);

        // Should map AB -> 1 + i
        auto reduced_ab = book.reduce(Polynomial{Monomial{5, 1.0}});
        EXPECT_EQ(reduced_ab, Polynomial::Scalar(std::complex{1.0, 1.0}));

        // Should map A -> 1
        EXPECT_EQ(book.reduce(Polynomial{Monomial{2}}), Polynomial::Scalar(std::complex{1.0, 0.0}));
    }

    TEST_F(Symbolic_MomentRulebook, Complete_RealAndReal) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        // Add rule to book: Re(ab) = a
        book.add_raw_rule(factory({Monomial{5, 0.5}, Monomial{5, 0.5, true}, Monomial{2, -1.0}}));
        book.complete();
        ASSERT_EQ(book.size(), 1);

        // Add another rule afterwards: Re(ab) = b
        book.add_raw_rule(factory({Monomial{5, -0.5}, Monomial{5, -0.5, true}, Monomial{3, 1.0}}));
        book.complete();

        ASSERT_EQ(book.size(), 2);

        // Should map AB -> iIm(AB) + a
        auto reduced_ab = book.reduce(Polynomial{Monomial{5, 1.0}});
        EXPECT_EQ(reduced_ab, factory({Monomial{5, 0.5}, Monomial{5, -0.5, true}, Monomial{2, 1.0}}));

        // Should map b -> a
        EXPECT_EQ(book.reduce(Polynomial{Monomial{3}}), Polynomial(Monomial{2}));
    }

    TEST_F(Symbolic_MomentRulebook, Complete_RealAndSkew) {
        const std::complex<double> skew_direction{1.0 / std::numbers::sqrt2, 1.0 / std::numbers::sqrt2}; // pi / 4
        const std::complex<double> offskew_direction{-1.0 / std::numbers::sqrt2, 1.0 / std::numbers::sqrt2}; // 3 pi / 4

        // Add real rule to book
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;
        book.inject(factory, 5, 1, Polynomial{Monomial{2, 1.0}}); // Rule should be Re(AB) -> <A>

        // Make skew rule
        const Polynomial skew_poly = factory({{Monomial{5, 0.5 * std::conj(skew_direction)},
                                         Monomial{5, 0.5 * skew_direction, true},
                                         Monomial{3, -1.0}}});
        ASSERT_EQ(MomentRule::get_difficulty(skew_poly),
                  MomentRule::PolynomialDifficulty::NonorientableRule);

        // Check direct version of rule
        MomentRule direct_skew_rule{factory, Polynomial{skew_poly}};
        EXPECT_TRUE(direct_skew_rule.is_partial());
        EXPECT_TRUE(approximately_equal(direct_skew_rule.partial_direction(), skew_direction, factory.zero_tolerance))
                    << direct_skew_rule.partial_direction();
        EXPECT_FALSE(direct_skew_rule.split().has_value());
        expect_matching_polynomials("Reduction of direct_skew_rule",
                                    direct_skew_rule.reduce(factory, Monomial{5, 1.0}),
                                    factory({Monomial{3, skew_direction},
                                             Monomial{5, 0.5},
                                             Monomial{5, -0.5 * skew_direction * skew_direction, true}}),
                                    factory.zero_tolerance);

        // Incorporate new rule
        book.add_raw_rule(Polynomial{skew_poly});
        book.complete();
        EXPECT_EQ(book.size(), 1);
        const auto& complete_rule = book.begin()->second;
        EXPECT_EQ(complete_rule.LHS(), 5);
        EXPECT_FALSE(complete_rule.is_partial());

        EXPECT_EQ(complete_rule.RHS(),
                  factory({Monomial{2, std::complex{1.0, -1.0}},
                           Monomial{3, std::complex{0.0, std::numbers::sqrt2}}}));
    }

    TEST_F(Symbolic_MomentRulebook, Complete_SkewAndReal) {
        const std::complex<double> skew_direction{1.0 / std::numbers::sqrt2, 1.0 / std::numbers::sqrt2}; // pi / 4
        const std::complex<double> offskew_direction{-1.0 / std::numbers::sqrt2, 1.0 / std::numbers::sqrt2}; // 3 pi / 4

        // Add skew rule to book
        MomentRulebook book{this->get_system()};

        const auto& factory = book.factory;
        book.inject(factory, 5, skew_direction, Polynomial{Monomial{3, 1.0}}); // Rule should be Kd(AB) -> <B>

        // Make real rule
        const Polynomial real_poly = factory({{Monomial{5, 0.5},
                                               Monomial{5, 0.5, true},
                                               Monomial{2, -1.0}}});
        // Incorporate new rule
        book.add_raw_rule(Polynomial{real_poly});
        book.complete();
        EXPECT_EQ(book.size(), 1);
        const auto& complete_rule = book.begin()->second;
        EXPECT_EQ(complete_rule.LHS(), 5);
        EXPECT_FALSE(complete_rule.is_partial());

        expect_matching_polynomials("Complete Rule",
                                    complete_rule.RHS(),
                                    factory({Monomial{2, std::complex{1.0, -1.0}},
                                             Monomial{3, std::complex{0.0, std::numbers::sqrt2}}}),
                                    factory.zero_tolerance);
    }



    TEST_F(Symbolic_MomentRulebook, Complete_FromMap) {
        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        std::map<symbol_name_t, double> raw_assignments;
        raw_assignments.insert(std::make_pair(2, 0.0)); // <a> = 0
        raw_assignments.insert(std::make_pair(3, 1.5)); // <b> = 1.5

        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());

        book.add_raw_rules(raw_assignments);
        book.complete();

        assert_matching_rules(book, {MomentRule{2, Polynomial()},
                                     MomentRule{3, Polynomial::Scalar(1.5)}});

    }



    TEST_F(Symbolic_MomentRulebook, CombineAndComplete_IntoEmpty) {
        // System
        MomentRulebook empty_book{this->get_system()};

        // Prepare rulebook
        MomentRulebook book{this->get_system()};
        const auto& factory = book.factory;

        std::map<symbol_name_t, double> raw_assignments;
        raw_assignments.insert(std::make_pair(2, 0.0)); // <a> = 0
        raw_assignments.insert(std::make_pair(3, 1.5)); // <b> = 1.5
        EXPECT_EQ(&book.symbols, &this->get_symbols());
        EXPECT_TRUE(book.empty());

        book.add_raw_rules(raw_assignments);
        book.complete();

        assert_matching_rules(book, {MomentRule{2, Polynomial()},
                                     MomentRule{3, Polynomial::Scalar(1.5)}});

        size_t new_rules = empty_book.combine_and_complete(std::move(book));
        EXPECT_EQ(new_rules, 2);
        assert_matching_rules(empty_book, {MomentRule{2, Polynomial()},
                                           MomentRule{3, Polynomial::Scalar(1.5)}});

    }

    TEST_F(Symbolic_MomentRulebook, CombineAndComplete_Trivial) {
        // System
        MomentRulebook book_one{this->get_system()};

        std::map<symbol_name_t, double> raw_assignments_one;
        raw_assignments_one.insert(std::make_pair(2, 0.0)); // <a> = 0


        book_one.add_raw_rules(raw_assignments_one);
        book_one.complete();

        assert_matching_rules(book_one, {MomentRule{2, Polynomial()}});

        // Prepare rulebook
        MomentRulebook book_two{this->get_system()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(3, 1.5)); // <b> = 1.5
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();

        assert_matching_rules(book_two, {MomentRule{3, Polynomial::Scalar(1.5)}});

        size_t new_rules = book_one.combine_and_complete(std::move(book_two));
        EXPECT_EQ(new_rules, 1);
        assert_matching_rules(book_one, {MomentRule{2, Polynomial()},
                                         MomentRule{3, Polynomial::Scalar(1.5)}});

    }

    TEST_F(Symbolic_MomentRulebook, CombineAndComplete_WithRewrite) {
        // Prepare first rulebook <AA> -> <A>
        MomentRulebook book_one{this->get_system()};
        auto& factory = this->get_factory();

        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(2, -0.5)})); // <aa> - 0.5<a> = 0
        book_one.add_raw_rules(std::move(raw_combos_one));
        book_one.complete();

        assert_matching_rules(book_one, {MomentRule{4, factory({Monomial(2, 0.5)})}});

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_two{this->get_system()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(2, 0.5)); // <a> = 0.5
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();
        assert_matching_rules(book_two, {MomentRule{2, Polynomial::Scalar(0.5)}});

        size_t new_rules = book_one.combine_and_complete(std::move(book_two));
        EXPECT_EQ(new_rules, 1);
        assert_matching_rules(book_one, {MomentRule{2, Polynomial::Scalar(0.5)},
                                         MomentRule{4, Polynomial::Scalar(0.25)}});

    }

    TEST_F(Symbolic_MomentRulebook, CombineAndComplete_FailBadRule) {
        // Prepare first rulebook <AA> -> <A>
        MomentRulebook book_one{this->get_system()};
        auto& factory = this->get_factory();

        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_one.add_raw_rules(std::move(raw_combos_one));
        book_one.complete();

        assert_matching_rules(book_one, {MomentRule{4, Polynomial::Scalar(0.5)}});

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_two{this->get_system()};
        std::map<symbol_name_t, double> raw_assignments_two;
        raw_assignments_two.insert(std::make_pair(4, 0.25)); // <aa> = 0.25
        book_two.add_raw_rules(raw_assignments_two);
        book_two.complete();
        assert_matching_rules(book_two, {MomentRule{4, Polynomial::Scalar(0.25)}});

        EXPECT_THROW(book_one.combine_and_complete(std::move(book_two)), errors::invalid_moment_rule);

        assert_matching_rules(book_one, {MomentRule{4, Polynomial::Scalar(0.5)}});
        EXPECT_FALSE(book_one.pending_rules());

    }

    TEST_F(Symbolic_MomentRulebook, CloneMomentMatrix) {
        // Build unlinked pair (uninflated)
        auto& ams = this->get_system();
        const auto& context = this->get_context();
        auto& symbols = this->get_symbols();

        const auto& factory = this->get_factory();

        // Get operator names
        ASSERT_EQ(context.size(), 2);
        oper_name_t op_a = 0;
        oper_name_t op_b = 1;

        // Make moment matrix, then find symbols
        auto [mm_id, moment_matrix] = ams.MomentMatrix.create(1);
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
                   Monomial{id_a}, Monomial{id_aa}, Monomial{id_ab, 1.0, true},
                   Monomial{id_b}, Monomial{id_ab}, Monomial{id_bb}};
        MonomialMatrix ref_mm{context, symbols, factory.zero_tolerance,
                              std::make_unique<SquareMatrix<Monomial>>(3, std::move(ref_mm_data)), true};

        compare_symbol_matrices(moment_matrix, ref_mm, "Moment matrix");

        // Build substitutions of just A
        auto [rb_id, book] = ams.Rulebook.add(std::make_unique<MomentRulebook>(this->get_system()));
        book.inject(id_a, Polynomial::Scalar(2.0)); // A -> 2
        book.inject(id_b, Polynomial::Scalar(3.0)); // B -> 3

        // Rewrite moment matrix with known values
        auto [sub_id, sub_matrix] = ams.SubstitutedMatrix.create(SubstitutedMatrixIndex(mm_id, rb_id));

        // Test matrix object is unique
        ASSERT_NE(mm_id, sub_id);
        ASSERT_NE(&moment_matrix, &sub_matrix);
        ASSERT_TRUE(sub_matrix.is_monomial());

        // Symbol matrix should have with a replaced by 2.0
        ASSERT_EQ(sub_matrix.Dimension(), 3);
        const auto& sub_symbols = dynamic_cast<const MonomialMatrix&>(sub_matrix).SymbolMatrix;
        EXPECT_EQ(sub_symbols(0, 0), Monomial(id_e));
        EXPECT_EQ(sub_symbols(0, 1), Monomial(id_e, 2.0));
        EXPECT_EQ(sub_symbols(0, 2), Monomial(id_e, 3.0));
        EXPECT_EQ(sub_symbols(1, 0), Monomial(id_e, 2.0));
        EXPECT_EQ(sub_symbols(1, 1), Monomial(id_aa, 1.0));
        EXPECT_EQ(sub_symbols(1, 2), Monomial(id_ab, 1.0));
        EXPECT_EQ(sub_symbols(2, 0), Monomial(id_e, 3.0));
        EXPECT_EQ(sub_symbols(2, 1), Monomial(id_ab, 1.0, true));
        EXPECT_EQ(sub_symbols(2, 2), Monomial(id_bb));

        // Check aliasing/caching
        const auto& sub_matrix_alias = ams.SubstitutedMatrix(SubstitutedMatrixIndex(mm_id, rb_id));
        EXPECT_EQ(&sub_matrix_alias.context, &context);
        ASSERT_EQ(&sub_matrix_alias, &sub_matrix);
    }


    TEST_F(Symbolic_MomentRulebook, FirstNoncontainedRule_BEmpty) {
        // Prepare first rulebook <A> -> 0.5
        MomentRulebook book_A{this->get_system()};
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_A.add_raw_rules(std::move(raw_combos_one));
        book_A.complete();

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_B{this->get_system()};
        book_B.complete();
        EXPECT_TRUE(book_B.empty());

        const auto* AfncrB = book_A.first_noncontained_rule(book_B);
        EXPECT_EQ(AfncrB, nullptr);

        const auto* BfncrA = book_B.first_noncontained_rule(book_A);
        ASSERT_NE(BfncrA, nullptr);
        EXPECT_EQ(BfncrA->LHS(), 4);

        auto [res, inAnotInB, inBnotInA] = book_A.compare_rulebooks(book_B);
        EXPECT_EQ(res, MomentRulebook::RulebookComparisonResult::AContainsB);
        EXPECT_EQ(inAnotInB, BfncrA);
        EXPECT_EQ(inBnotInA, nullptr);
    }

    TEST_F(Symbolic_MomentRulebook, FirstNoncontainedRule_AEqualsB) {
        // Prepare first rulebook <A> -> 0.5
        MomentRulebook book_A{this->get_system()};
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_A.add_raw_rules(std::move(raw_combos_one));
        book_A.complete();

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_B{this->get_system()};
        std::vector<Polynomial> raw_combos_two;
        raw_combos_two.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_B.add_raw_rules(std::move(raw_combos_two));
        book_B.complete();

        const auto* AsupersetB = book_A.first_noncontained_rule(book_B);
        EXPECT_EQ(AsupersetB, nullptr);

        const auto* BsupersetA = book_B.first_noncontained_rule(book_A);
        EXPECT_EQ(BsupersetA, nullptr);

        auto [res, inAnotInB, inBnotInA] = book_A.compare_rulebooks(book_B);
        EXPECT_EQ(res, MomentRulebook::RulebookComparisonResult::AEqualsB);
        EXPECT_EQ(inAnotInB, nullptr);
        EXPECT_EQ(inBnotInA, nullptr);
    }

    TEST_F(Symbolic_MomentRulebook, FirstNoncontainedRule_AsupersetB) {
        // Prepare first rulebook <A> -> 0.5
        MomentRulebook book_A{this->get_system()};
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial{1, -0.5}})); // <aa> - 0.5 = 0
        raw_combos_one.emplace_back(factory({Monomial(3, 1.0), Monomial{1, -2.0}})); // <b> - 2.0 = 0
        book_A.add_raw_rules(std::move(raw_combos_one));
        book_A.complete();

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_B{this->get_system()};
        std::vector<Polynomial> raw_combos_two;
        raw_combos_two.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_B.add_raw_rules(std::move(raw_combos_two));
        book_B.complete();

        const auto* AsupersetB = book_A.first_noncontained_rule(book_B);
        EXPECT_EQ(AsupersetB, nullptr);

        const auto* BsupersetA = book_B.first_noncontained_rule(book_A);
        ASSERT_NE(BsupersetA, nullptr);
        EXPECT_EQ(BsupersetA->LHS(), 3);

        auto [res, inAnotInB, inBnotInA] = book_A.compare_rulebooks(book_B);
        EXPECT_EQ(res, MomentRulebook::RulebookComparisonResult::AContainsB);
        EXPECT_EQ(inAnotInB, BsupersetA);
        EXPECT_EQ(inBnotInA, nullptr);


        auto [rev_res, rev_inAnotInB, rev_inBnotInA] = book_B.compare_rulebooks(book_A);
        EXPECT_EQ(rev_res, MomentRulebook::RulebookComparisonResult::BContainsA);
        EXPECT_EQ(rev_inAnotInB, nullptr);
        EXPECT_EQ(rev_inBnotInA, BsupersetA);
    }

    TEST_F(Symbolic_MomentRulebook, FirstNoncontainedRule_AdisjointB_One) {
        // Prepare first rulebook <A> -> 0.5
        MomentRulebook book_A{this->get_system()};
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(3, 1.0), Monomial{1, -2.0}})); // <b> - 2.0 = 0
        book_A.add_raw_rules(std::move(raw_combos_one));
        book_A.complete();

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_B{this->get_system()};
        std::vector<Polynomial> raw_combos_two;
        raw_combos_two.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_B.add_raw_rules(std::move(raw_combos_two));
        book_B.complete();

        const auto* AsupersetB = book_A.first_noncontained_rule(book_B);
        ASSERT_NE(AsupersetB, nullptr);
        EXPECT_EQ(AsupersetB->LHS(), 4);

        const auto* BsupersetA = book_B.first_noncontained_rule(book_A);
        ASSERT_NE(BsupersetA, nullptr);
        EXPECT_EQ(BsupersetA->LHS(), 3);

        auto [res, inAnotInB, inBnotInA] = book_A.compare_rulebooks(book_B);
        EXPECT_EQ(res, MomentRulebook::RulebookComparisonResult::Disjoint);
        EXPECT_EQ(inAnotInB, BsupersetA);
        EXPECT_EQ(inBnotInA, AsupersetB);
    }

    TEST_F(Symbolic_MomentRulebook, FirstNoncontainedRule_AdisjointB_Contradict) {
        // Prepare first rulebook <A> -> 0.5
        MomentRulebook book_A{this->get_system()};
        auto& factory = this->get_factory();
        std::vector<Polynomial> raw_combos_one;
        raw_combos_one.emplace_back(factory({Monomial(4, 1.0), Monomial{1, -2.0}})); // <b> - 2.0 = 0
        book_A.add_raw_rules(std::move(raw_combos_one));
        book_A.complete();

        // Prepare second rulebook <A> -> 0.5
        MomentRulebook book_B{this->get_system()};
        std::vector<Polynomial> raw_combos_two;
        raw_combos_two.emplace_back(factory({Monomial(4, 1.0), Monomial(1, -0.5)})); // <aa> - 0.5 = 0
        book_B.add_raw_rules(std::move(raw_combos_two));
        book_B.complete();

        const auto* AsupersetB = book_A.first_noncontained_rule(book_B);
        ASSERT_NE(AsupersetB, nullptr);
        EXPECT_EQ(AsupersetB->LHS(), 4);
        EXPECT_EQ(AsupersetB->RHS(), Polynomial::Scalar(0.5));

        const auto* BsupersetA = book_B.first_noncontained_rule(book_A);
        ASSERT_NE(BsupersetA, nullptr);
        EXPECT_EQ(BsupersetA->LHS(), 4);
        EXPECT_EQ(BsupersetA->RHS(), Polynomial::Scalar(2.0));

        auto [res, inAnotInB, inBnotInA] = book_A.compare_rulebooks(book_B);
        EXPECT_EQ(res, MomentRulebook::RulebookComparisonResult::Disjoint);
        EXPECT_EQ(inAnotInB, BsupersetA);
        EXPECT_EQ(inBnotInA, AsupersetB);
    }

    TEST_F(Symbolic_MomentRulebook, SubstitutedMatrix_NotFound) {
        const auto& system = this->get_system();
        EXPECT_THROW([[maybe_unused]] const auto& sm = system.SubstitutedMatrix({5, 5}),
                     Moment::errors::missing_component);

        auto [mm_id, moment_matrix] = this->get_system().MomentMatrix.create(1);

        EXPECT_THROW([[maybe_unused]] const auto& sm2 = system.SubstitutedMatrix({static_cast<ptrdiff_t>(mm_id), 5}),
                     Moment::errors::missing_component);

    }
}
